require_relative '../../zemu/lib/zemu'

# Status LEDs object
#
# Represents the status LEDs on the z80 computer.
class StatusLedPort < Zemu::Config::IOPort
    def initialize
        super

        when_setup do
<<-EOF
#include <stdio.h>

const char * #{name}_labels[8] = { #{labels.map { |x| "\"#{x}\"" }.join(", ")} };
zuint8 #{name}_reg_value = 0;

zuint8 zemu_io_#{name}_value(void)
{
    return #{name}_reg_value;
}
EOF
        end

        # Cannot read from port, but we need
        # to return a value.
        when_read do
<<-EOF
if (port == #{port}) return 0;
EOF
        end

        # Write just prints the new
        # value of the LEDs to stdout.
        when_write do
<<-EOF
if (port == #{port})
{
    #{name}_reg_value = value;

#ifndef TEST
    for (int i = 0; i < 8; i++)
    {
        printf("%-4s", #{name}_labels[i]);
    }
    printf("\\n");
    for (int i = 0; i < 8; i++)
    {
        if (value & 0x80) printf("1   ");
        else printf("0   ");
        value <<= 1;
    }
    printf("\\n\\n");
#endif
}
EOF
        end

        when_clock do
        end
    end

    def functions
        [
            {
                "name" => "zemu_io_#{name}_value".to_sym,
                "args" => [],
                "return" => :uint8
            }
        ]
    end

    def params
        %w(name port labels)
    end
end

# Serial Input/Output object
#
# Represents a serial connection between the emulated CPU
# and the host machine, with input and output mapped to Z80 I/O
# ports.
class Serial6850 < Zemu::Config::IOPort
    # Constructor.
    #
    # Takes a block in which the parameters of the serial port
    # can be initialized.
    #
    # All parameters can be set within this block.
    # They become readonly as soon as the block completes.
    #
    # @example
    #   
    #   Zemu::Config::SerialPort.new do
    #       name "serial"
    #       in_port 0x00
    #       out_port 0x01
    #   end
    #
    #
    def initialize
        super

        when_setup do
            "#include <stdio.h>\n" +
            "zuint8 io_#{name}_tx;\n" +
            "zuint8 io_#{name}_rx;\n" +
            "zuint8 io_#{name}_status;\n" +
            "zuint8 io_#{name}_control;\n" +
            "zboolean int_#{name}_tx = FALSE;\n" +
            "zboolean int_#{name}_rx = FALSE;\n" +
            "\n" +
            "zusize zemu_io_#{name}_buffer_size(void)\n" +
            "{\n" +
            "    if (io_#{name}_status & 0x02) return 0;\n" +
            "    else return 1;\n" +
            "}\n" +
            "\n" +
            "void zemu_io_#{name}_master_puts(zuint8 val)\n" +
            "{\n" +
            "    io_#{name}_rx = val;\n" +
            "    io_#{name}_status |= 0x01;\n" +
            "    int_#{name}_rx = TRUE;\n" +
            "}\n" +
            "\n" +
            "zuint8 zemu_io_#{name}_master_gets(void)\n" +
            "{\n" +
            "    io_#{name}_status |= 0x02;\n" +
            "    return io_#{name}_tx;\n" +
            "}\n"
        end

        when_read do
            "if (port == #{data_port})\n" +
            "{\n" +
            "    io_#{name}_status &= ~0x01;\n" +
            "    int_#{name}_rx = FALSE;\n" +
            "    return io_#{name}_rx;\n" +
            "}\n" +
            "else if (port == #{control_port})\n" +
            "{\n" +
            "    return io_#{name}_status;\n" +
            "}\n"
        end

        when_write do
            "if (port == #{data_port})\n" +
            "{\n" +
            "    io_#{name}_status &= ~0x02;\n" +
            "    io_#{name}_tx = value;\n" +
            "}\n" +
            "else if (port == #{control_port})\n" +
            "{\n" +
            "    if ((value & 0x03) == 0x03) io_#{name}_status = 0x02;\n" +
            "    else io_#{name}_control = value;\n" +
            "}\n"
        end

        when_clock do
            "if ((io_#{name}_control & 0x20) && (io_#{name}_status & 0x02)) int_#{name}_tx = TRUE;\n" +
            "else int_#{name}_tx = FALSE;\n" +
            "if (int_#{name}_rx || int_#{name}_tx) { io_#{name}_status |= 0x80; zemu_io_int_on(instance); }\n" +
            "else { io_#{name}_status &= ~0x80; zemu_io_int_off(instance); }\n"
        end
    end

    # Defines FFI API which will be available to the instance wrapper if this IO device is used.
    def functions
        [
            {"name" => "zemu_io_#{name}_master_puts".to_sym, "args" => [:uint8], "return" => :void},
            {"name" => "zemu_io_#{name}_master_gets".to_sym, "args" => [], "return" => :uint8},
            {"name" => "zemu_io_#{name}_buffer_size".to_sym, "args" => [], "return" => :uint64}
        ]
    end

    # Valid parameters for a SerialPort, along with those
    # defined in [Zemu::Config::IOPort].
    def params
        super + %w(data_port control_port)
    end
end

def pad(array, size, value)
    if array.size >= size
        array
    else
        array + Array.new(size - array.size, value)
    end
end

def zemu_config
    conf = Zemu::Config.new do
        name "zemu_debug_interactive"

        clock_speed 3_686_400
        serial_delay 0.001

        add_memory (Zemu::Config::RAM.new do
            name "ram_kernel"
            address 0x0000
            size    0x6000
            
            # Pad with halts so we detect out-of-bounds accesses.
            contents pad(from_binary("kernel_debug.bin"), 0x2000, 0x76)
        end)

        add_memory (Zemu::Config::RAM.new do
            name "ram_cp"
            address 0x6000
            size    0x2000
            
            # Pad with halts so we detect out-of-bounds accesses.
            contents pad(from_binary("command.bin"), 0x2000, 0x76)
        end)

        add_memory (Zemu::Config::RAM.new do
            name "ram_user"
            address 0x8000
            size    0x8000

            # Pad with halts so we detect out-of-bounds accesses.
            contents pad([], 0x8000, 0x76)
        end)

        add_io (Serial6850.new do
            name "serial"
            control_port 0x00
            data_port 0x01
        end)

        add_io (Zemu::Config::BlockDrive.new do
            name "drive"
            base_port 0x18
            sector_size 512
            num_sectors 131072

            initialize_from "disk_copy.bin"
        end)

        add_io (StatusLedPort.new do
            name "status"
            port 0x80
            labels %w(X X X X X SYS INT MEM)
        end)
    end

    conf
end

def zemu_start
    config = zemu_config()

    Zemu.start_interactive(config, print_serial: false)
end

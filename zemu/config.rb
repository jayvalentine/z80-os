require_relative '../../zemu/lib/zemu'

# Status LEDs object
#
# Represents the status LEDs on the z80 computer.
class StatusLedPort < Zemu::Config::BusDevice
    def initialize
        super

        @display = true
        @register = 0
    end

    def set_display(state)
        @display = state
    end

    def io_write(port, value)
        if (port == io_port) && @display
            @register = value

            8.times do |i|
                s = "%-4s" % labels[i]
                print(s)
            end
            print("\n")

            8.times do |i|
                if (value & 0x80) == 0x80
                    print("0   ")
                else
                    print("1   ")
                end

                value = value << 1
            end
            print("\n\n")
        end
    end

    def register
        @register
    end

    def params
        super + %w(io_port labels)
    end
end

# Serial Input/Output object
#
# Represents a serial connection between the emulated CPU
# and the host machine, with input and output mapped to Z80 I/O
# ports.
class Serial6850 < Zemu::Config::BusDevice
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

        @buffer_rx = []
        @buffer_tx = []

        @status = 0
        @control = 0
        @int_rx = false
    end

    def transmitted_count
        @buffer_tx.size
    end

    def get_byte()
        @status |= 0x02
        return @buffer_tx.shift()
    end

    def put_byte(b)
        @buffer_rx = [b]
        @status |= 0x01
        @int_rx = true
    end

    def io_read(port)
        if port == data_port
            @status &= ~0x01
            @int_rx = false
            return @buffer_rx.shift()
        elsif port == control_port
            return @status
        end

        nil
    end

    def io_write(port, value)
        if port == data_port
            @status &= ~0x02
            @buffer_tx = [value]
        elsif port == control_port
            if ((value & 0x03) == 0x03)
                @status = 0x02
            else
                @control = value
            end
        end
    end

    def clock(cycles)
        if ((@control & 0x20) == 0x20) && ((@status & 0x02) == 0x02)
            int_tx = true
        else
            int_tx = false
        end

        if @int_rx || int_tx
            @status |= 0x80
            interrupt(true)
        else
            @status &= ~0x80
            interrupt(false)
        end
    end

    # Valid parameters for a Serial6850, along with those
    # defined in [Zemu::Config::BusDevice].
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
            io_port 0x80
            labels %w(X X X X X SYS INT MEM)
        end)
    end

    conf
end

def zemu_start
    config = zemu_config()

    Zemu.start_interactive(config, print_serial: false)
end

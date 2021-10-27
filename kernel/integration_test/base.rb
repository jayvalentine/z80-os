require 'minitest/test'
require 'fileutils'

require_relative '../../../zemu/lib/zemu'
require_relative '../../../z80-libraries/vars.rb'

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

class IntegrationTest < Minitest::Test
    def compile_test_code(test_files, output_name)
        cmd = "zcc "
        cmd += "+#{CONFIG} -compiler-sccz80 "
        cmd += "-O2 -SO2 "
        cmd += "-L#{LIB} -I#{LIB_INCLUDE} "
        cmd += "-Ca\"-I#{LIB_INCLUDE}\" "
        cmd += "-Cl\"-r0x6000\" "
        cmd += "-crt0 command/reset.asm "
        cmd += "-lstdlib "
        cmd += "-m "
        cmd += "-o #{output_name} "
        cmd += test_files.join(" ")

        success = system(cmd)

        assert success, "Failed to build test code!"
    end

    def start_instance(binary)
        binary_name = File.basename(binary, ".bin")

        # Create copy of the "master" disk image for this test case.
        disk_file_name = "#{binary_name}_disk.bin"
        FileUtils.cp "kernel/integration_test/disk.bin", disk_file_name

        conf = Zemu::Config.new do
            name "zemu_#{binary_name}"
    
            clock_speed 3_686_400
            serial_delay 0.001
    
            add_memory (Zemu::Config::RAM.new do
                name "ram_kernel"
                address 0x0000
                size    0x6000
                
                contents from_binary("kernel_debug.bin")
            end)

            add_memory (Zemu::Config::RAM.new do
                name "ram_cp"
                address 0x6000
                size    0x2000
                
                contents from_binary(binary)
            end)
    
            add_memory (Zemu::Config::RAM.new do
                name "ram_user"
                address 0x8000
                size    0x7FFF
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

                initialize_from disk_file_name
            end)
        end

        @instance = Zemu.start(conf)
    end

    def teardown
        @instance.quit unless @instance.nil?
    end
end

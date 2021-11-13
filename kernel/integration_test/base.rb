require 'minitest/test'
require 'fileutils'

require_relative '../../../zemu/lib/zemu'
require_relative '../../../z80-libraries/vars.rb'

require_relative '../../zemu/config'

class IntegrationTest < Minitest::Test
    def compile_test_code(test_files, output_name)
        cmd = "zcc "
        cmd += "+#{CONFIG} -compiler-sccz80 "
        cmd += "-O2 -SO2 "
        cmd += "-L#{LIB} -I#{LIB_INCLUDE} "
        cmd += "-Ca\"-I#{LIB_INCLUDE}\" "
        cmd += "-Cl\"-r0x6000\" "
        cmd += "-crt0 kernel/integration_test/reset.asm "
        cmd += "-lstdlib "
        cmd += "-m "
        cmd += "-o #{output_name} "
        cmd += test_files.join(" ")

        success = system(cmd)

        assert success, "Failed to build test code!"

        system("z88dk-dis -o 0x6000 #{output_name} > #{output_name}.diss")
    end

    def compile_user_code(address, test_files, output_name)
        address_string = "0x%04X" % address
        cmd = "zcc "
        cmd += "+#{CONFIG} -compiler-sccz80 "
        cmd += "-O2 -SO2 "
        cmd += "-L#{LIB} -I#{LIB_INCLUDE} "
        cmd += "-Ca\"-I#{LIB_INCLUDE}\" "
        cmd += "-Cl\"-r#{address_string}\" "
        cmd += "-crt0 #{CRT0} "
        cmd += "-lstdlib "
        cmd += "-m "
        cmd += "-o #{output_name} "
        cmd += test_files.join(" ")

        success = system(cmd)

        assert success, "Failed to build user program (#{output_name})!"

        system("z88dk-dis -o #{address_string} #{output_name} > #{output_name}.diss")
    end

    def start_instance(binary)
        binary_name = File.basename(binary, ".bin")

        # Create copy of the "master" disk image for this test case.
        disk_file_name = "#{binary_name}_disk.bin"
        FileUtils.cp "kernel/integration_test/disk.img", disk_file_name

        conf = Zemu::Config.new do
            name "zemu_#{binary_name}"
    
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
                contents pad(from_binary(binary), 0x2000, 0x76)
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

                initialize_from disk_file_name
            end)
        end

        @instance = Zemu.start(conf)
    end

    def load_user_program(address, program)
        File.open(program, "rb") do |f|
            # Load maximum of 2k
            2048.times do
                b = f.getbyte
                break if b.nil?

                @instance.set_memory(address, b)
                address += 1
            end
        end
    end

    def teardown
        @instance.quit unless @instance.nil?
    end

    def print_stack
        sp = @instance.registers["SP"]
        8.times do |i|
            offset = i * 2
            lo_addr = sp + offset
            hi_addr = lo_addr + 1
            
            lo = @instance.memory(lo_addr)
            hi = @instance.memory(hi_addr)

            val = (hi << 8) | lo
            
            mem_val_lo = @instance.memory(val)
            mem_val_hi = @instance.memory(val+1)
            mem_val = (mem_val_hi << 8) | mem_val_lo

            puts ("SP+#{offset}\t%04x (%04x)" % [val, mem_val])
        end
    end
end

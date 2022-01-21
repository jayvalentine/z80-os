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
        cmd += "-Cl\"-r0x8000\" "
        cmd += "-crt0 kernel/integration_test/reset.asm "
        cmd += "-lstdlib "
        cmd += "-m "
        cmd += "-o #{output_name} "
        cmd += test_files.join(" ")

        success = system(cmd)

        assert success, "Failed to build test code!"

        system("z88dk-dis -o 0x8000 #{output_name} > #{output_name}.diss")
    end

    def compile_user_code(address, test_files, output_name)
        address_string = "0x%04X" % address
        cmd = "zcc "
        cmd += "+#{CONFIG} -compiler-sccz80 "
        cmd += "-O2 -SO2 "
        cmd += "-L#{LIB} -I#{LIB_INCLUDE} "
        cmd += "-Ca\"-I#{LIB_INCLUDE}\" "
        cmd += "-Cl\"-r#{address_string}\" "
        cmd += "-crt0 #{PROCESS_CRT0} "
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

        conf = zemu_config(binary_name, binary, disk_file_name)

        @instance = Zemu.start(conf, TEST: 1)
        @instance.device("status").display_off
        @instance.device("timer").display_off
        @instance.device("banked_ram").display_off

        # We expect to start executing at 0x8000,
        # where the system process would reside normally.
        @instance.break 0x8000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue 2000000
        
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x8000, @instance.registers["PC"], "Breakpoint at wrong address (%04x)" % @instance.registers["PC"]
        assert_equal 0, @instance.device("banked_ram").bank, "wrong RAM bank!"

        @instance.remove_break 0x8000, :program

        # Set breakpoint at 0x0000 to catch unintentional resets.
        @instance.break 0x0000, :program
    end

    def load_user_program(address, program)
        offset = address - 0x8000
        File.open(program, "rb") do |f|
            # Load maximum of 256 bytes
            2048.times do
                b = f.getbyte
                break if b.nil?

                @instance.device("banked_ram").contents(0)[offset] = b
                offset += 1
            end
        end
    end

    def teardown
        @instance.quit unless @instance.nil?
    end

    def print_stack
        sp = @instance.registers["SP"]
        16.times do |i|
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

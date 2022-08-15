require 'minitest/test'

require 'fileutils'
require 'benchmark'
require 'time'

require_relative '../../../zemu/lib/zemu'
require_relative '../../../z80-libraries/vars.rb'

require_relative '../../zemu/config'

class IntegrationTest < Minitest::Test
    def log(message)
        File.open("#{@test_name}.log", "a") do |f|
            f.puts message
        end
    end

    def setup
        @test_name = @NAME

        File.open("#{@test_name}.log", "w+") do |f|
            f.puts "Log for #{@test_name}"
        end

        # Compile code for the test case.
        compile_time = Benchmark.realtime do
            source_files = Dir.glob("kernel/integration_test/#{@test_name}/*.c") + Dir.glob("kernel/integration_test/#{@test_name}/*.asm")
            compile_test_code(source_files, "kernel/integration_test/#{@test_name}/#{@test_name}.bin")
        end

        log("compile:        #{compile_time}s")

        # Start an instance of the compiled test binary.
        start_instance("kernel/integration_test/#{@test_name}/#{@test_name}.bin")

        @test_start_time = Time.now
    end

    def teardown
        unless @test_start_time.nil?
            test_time = Time.now - @test_start_time
            log("test:           #{test_time}s")
        end

        # Get instructions executed by the test.
        unless @instance.nil?
            File.open("#{@test_name}.coverage", "w+") do |f|
                @instance.addrs.each do |a|
                    f.puts ("$%04x" % a)
                end
            end


            @instance.quit
        end
    end

    def compile(test_files, output_name, crt0, address)
        address_string = "%04x" % address

        object_files = []
        test_files.each do |t|
            if t.end_with? ".asm"
                obj = File.basename(t, ".asm") + ".rel"
                
                cmd = "sdasz80 -plosgffw "
                cmd += "#{obj} "
                cmd += t
                success = system(cmd)
                assert success, "Failed to assemble #{t}"
                
                object_files << obj
            else
                obj = File.basename(t, ".c") + ".rel"
                
                cmd = "sdcc -mz80 -c --std-sdcc99 "
                cmd += "-I#{LIB_INCLUDE} "
                cmd += "-o #{obj} "
                cmd += t
                success = system(cmd)
                assert success, "Failed to compile #{t}"
                FileUtils.rm("#{File.basename(t, ".c")}.asm")
                
                object_files << obj
            end
        end
        
        cmd = "sdcc -mz80 --no-std-crt0 "
        cmd += "-Wl-b_CODE=0x%04x " % address
        cmd += "-Wl-b_DATA=0x%04x " % (address + 0x1000)
        cmd += "-o #{File.basename(output_name, ".bin")}.hex "
        cmd += "-L #{LIB} "
        cmd += crt0 + " "
        cmd += object_files.join(" ")
        cmd += " stdlib.lib"
        
        success = system("#{cmd} > link.log 2>&1")
        
        assert success, "Failed to link test code!:\n#{cmd}\n#{File.read("link.log")}"
        
        system("objcopy --gap-fill 0x76 --pad-to 0xf000 -Iihex -Obinary #{File.basename(output_name, ".bin")}.hex #{output_name}")
        system("z88dk-dis -o 0x#{address_string} #{output_name} > #{output_name}.diss")
    end
    
    def compile_test_code(test_files, output_name)
        compile(test_files, output_name, "kernel/integration_test/reset.rel", 0x8000)
    end

    def compile_user_code(address)
        test_files = Dir.glob("kernel/integration_test/#{@test_name}/user/*.c")

        output_name = "kernel/integration_test/#{@test_name}/user/program.bin"

        compile(test_files, output_name, "#{LIB}/process_crt0.rel", address)
    end

    def start_instance(binary)
        binary_name = File.basename(binary, ".bin")

        # Create copy of the "master" disk image for this test case.
        disk_file_name = "#{binary_name}_disk.bin"
        FileUtils.cp "kernel/integration_test/disk.img", disk_file_name

        conf = nil
        create_config_time = Benchmark.realtime do
            conf = zemu_config(binary_name, binary, disk_file_name)
        end

        log("create config:  #{create_config_time}s")

        start_instance_time = Benchmark.realtime do
            @instance = Zemu.start(conf, TEST: 1)
        end

        log("start instance: #{start_instance_time}s")

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

    # Loads a user program into the first bank of memory at the given address.
    # This is space reserved in the test code that is then written to disk
    # to be loaded via the pload syscall.
    def load_user_program(address)
        program = "kernel/integration_test/#{@test_name}/user/program.bin"

        offset = address - 0x8000
        File.open(program, "rb") do |f|
            # Load maximum of 2048 bytes
            2048.times do
                b = f.getbyte
                break if b.nil?

                @instance.device("banked_ram").contents(0)[offset] = b
                offset += 1
            end
        end
    end

    def get_int16(addr)
        lo = @instance.memory(addr)
        hi = @instance.memory(addr + 1)
        (hi << 8) | lo
    end

    def get_int8(addr)
        @instance.memory(addr)
    end

    def get_string(addr)
        s = ""
        while (c = @instance.memory(addr)) != 0
            s += c.chr()
            addr += 1
        end
        s
    end

    def get_array(addr, size)
        s = ""
        while size > 0
            c = @instance.memory(addr)
            s += c.chr()
            addr += 1
            size -= 1
        end
        s
    end

    def load_test_map()
        Zemu::Debug.load_map("#{@test_name}.map")
    end

    def load_kernel_map()
        Zemu::Debug.load_map("kernel_debug.map")
    end

    def schedule_table
        #kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")
        addr = 0x6473

        16.times do |i|
            base = addr + (i * 5)
            state = get_int8(base)
            pid = get_int16(base+1)
            exitcode = get_int16(base+3)

            puts "SCHEDULE TABLE #{i}"
            puts "    STATE: %d" % state
            puts "    PID:   %d" % pid
            puts "    EXIT:  %d" % exitcode
        end
    end

    def get_stack
        sp = @instance.registers["SP"]
        stack = []
        16.times do |i|
            offset = i * 2
            lo_addr = sp + offset
            hi_addr = lo_addr + 1
            
            lo = @instance.memory(lo_addr)
            hi = @instance.memory(hi_addr)

            val = (hi << 8) | lo
            stack << val
        end
        stack
    end

    def pc_str
        "0x#{@instance.registers["PC"].to_s(16)}"
    end

    def assert_program_finished
        assert @instance.halted?, "Program should have halted (at #{pc_str})."
        assert_equal 0x8008, @instance.registers["PC"], "Halted at wrong address (at #{pc_str})."
    end
end

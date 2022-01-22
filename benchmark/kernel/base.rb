require 'fileutils'

require_relative '../../../z80-libraries/vars'

BENCHMARKS = File.join(__dir__, "src")

class KernelBenchmark
    def start_instance(binary)
        binary_name = File.basename(binary, ".bin")

        # Create copy of the "master" disk image for this test case.
        disk_file_name = "#{binary_name}_disk.bin"
        FileUtils.cp "kernel/integration_test/disk.img", disk_file_name

        conf = zemu_config(binary_name, binary, disk_file_name)

        instance = Zemu.start(conf, TEST: 1)
        instance.device("status").display_off
        instance.device("timer").display_off
        instance.device("banked_ram").display_off

        return instance
    end

    def compile_test_code(test_files, output_name)
        test_files = ["kernel/integration_test/reset.asm"] + test_files

        object_files = []
        test_files.each do |t|
            if t.end_with? ".asm"
                obj = File.basename(t, ".asm") + ".rel"
                
                cmd = "sdasz80 -plosgffw "
                cmd += "#{obj} "
                cmd += t
                success = system(cmd)
                
                object_files << obj
            else
                obj = File.basename(t, ".c") + ".rel"
                
                cmd = "sdcc -mz80 -c "
                cmd += "-I#{LIB_INCLUDE} "
                cmd += "-o #{obj} "
                cmd += t
                success = system(cmd)
                FileUtils.rm("#{File.basename(t, ".c")}.asm")
                
                object_files << obj
            end
        end
        
        cmd = "sdcc -mz80 --no-std-crt0 "
        cmd += "-Wl-b_CODE=0x8000 "
        cmd += "-Wl-b_DATA=0x9000 "
        cmd += "-o #{File.basename(output_name, ".bin")}.hex "
        cmd += "-L #{LIB} "
        cmd += object_files.join(" ")
        cmd += " stdlib.lib"
        
        success = system("#{cmd} > link.log 2>&1")
        
        system("objcopy --gap-fill 0x76 --pad-to 0xf000 -Iihex -Obinary #{File.basename(output_name, ".bin")}.hex #{output_name}")
        system("z88dk-dis -o 0x8000 #{output_name} > #{output_name}.diss")
    end
    
    def bench(num)
        total = 0
        min = 100_000_000
        max = 0

        num.times do
            val = yield
            total += val

            min = val if val < min
            max = val if val > max
        end

        avg = total.to_f / num.to_f
        avg = avg.round(2)
        min = min.round(2)
        max = max.round(2)

        [avg, min, max]
    end

    def benchmarks
        table = {}

        methods.each do |m|
            if /^benchmark_/ =~ m
                compile_test_code([File.join(BENCHMARKS, "#{m}.c")], "#{m}.bin")

                @instance = start_instance("#{m}.bin")
                avg, min, max = send(m)
                @instance.quit

                table[m] = [avg, min, max]
            end
        end

        puts("")
        
        headings = %w(benchmark avg min max)
        headings_str = "%-30s" % headings[0]
        headings[1..].each do |h|
            headings_str += "%-10s" % h
        end

        puts(headings_str)
        puts("-" * 60)

        table.each do |b, r|
            row = "%-30s" % b
            r.each do |v|
                row += "%-10s" % v.to_s
            end

            puts(row)
        end
    end
end

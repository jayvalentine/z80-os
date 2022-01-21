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

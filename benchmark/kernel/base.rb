require 'fileutils'

require_relative '../../../z80-libraries/vars'

BENCHMARKS = File.join(__dir__, "src")

class KernelBenchmark
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

            add_io (StatusLedPort.new do
                name "status"
                port 0x80
                labels %w(X X X X X SYS INT MEM)
            end)
        end

        return Zemu.start(conf, TEST: 1)
    end

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

        system("z88dk-dis -o 0x6000 #{output_name} > #{output_name}.diss")
    end
    
    def bench(num)
        total = 0
        num.times do
            total += yield
        end

        avg = total.to_f / num.to_f
        avg = avg.round(2)

        avg
    end

    def benchmarks
        table = {}

        methods.each do |m|
            if /^benchmark_/ =~ m
                compile_test_code([File.join(BENCHMARKS, "#{m}.c")], "#{m}.bin")

                @instance = start_instance("#{m}.bin")
                result = send(m)
                @instance.quit

                table[m] = result
            end
        end

        headings = %w(benchmark result)
        puts("")
        puts(headings.map { |h| "%-30s" % h }.join(""))
        puts("-" * 60)
        table.each do |b, r|
            puts("%-30s%-30s" % [b, r.to_s])
        end
    end
end

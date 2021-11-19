require 'fileutils'

require_relative '../../../z80-libraries/vars.rb'

BENCHMARKS = File.join(__dir__, "src")

module InterruptBenchmarks
    def self.start_instance(binary)
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

    def self.compile_test_code(test_files, output_name)
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

    def self.benchmark_interrupt_tx
        name = __method__.to_s
        puts "BENCHMARK: #{name}"

        compile_test_code([File.join(BENCHMARKS, "#{name}.c")], "#{name}.bin")

        instance = start_instance("#{name}.bin")

        # Get symbols.
        symbols = []
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")
        kernel_symbols.each do |addr, syms|
            syms.each do |sym|
                symbols << sym
            end
        end

        int_breakpoint_start = symbols.select { |s| s.label == "_interrupt_handler" }[0].address
        int_breakpoint_end = symbols.select { |s| s.label == "__interrupt_handler_end" }[0].address

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        instance.continue
        instance.remove_break 0x6000, :program

        # Set a breakpoint at the start and end of the ISR.
        instance.break int_breakpoint_start, :program
        instance.break int_breakpoint_end, :program

        1.times do |i|
            instance.continue 10000
            isr_cycles = instance.continue 10000
            
            puts "    Iteration #{i}: ISR runtime: #{isr_cycles} cycles"
        end
    end

    def self.benchmark_interrupt_rx
        name = __method__.to_s
        puts "BENCHMARK: #{name}"

        compile_test_code([File.join(BENCHMARKS, "#{name}.c")], "#{name}.bin")

        instance = start_instance("#{name}.bin")

        # Get symbols.
        symbols = []
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")
        kernel_symbols.each do |addr, syms|
            syms.each do |sym|
                symbols << sym
            end
        end

        int_breakpoint_start = symbols.select { |s| s.label == "_interrupt_handler" }[0].address
        int_breakpoint_end = symbols.select { |s| s.label == "__interrupt_handler_end" }[0].address

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        instance.continue
        instance.remove_break 0x6000, :program

        # Set a breakpoint at the start and end of the ISR.
        instance.break int_breakpoint_start, :program
        instance.break int_breakpoint_end, :program

        5.times do |i|
            # Write a character to the serial port.
            instance.serial_puts "A"

            # Run, we should hit the ISR.
            instance.continue 10000

            # Run again until we exit the ISR.
            isr_cycles = instance.continue 10000
            
            puts "    Iteration #{i}: ISR runtime: #{isr_cycles} cycles"
        end
    end
end

def benchmarks
    InterruptBenchmarks.benchmark_interrupt_tx()
    InterruptBenchmarks.benchmark_interrupt_rx()
end

require_relative 'base'

class InterruptBenchmarks < KernelBenchmark
    def benchmark_interrupt_tx
        # Get symbols.
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_breakpoint_start = kernel_symbols.find_by_name("_interrupt_handler").address
        int_breakpoint_end = kernel_symbols.find_by_name("__interrupt_handler_end").address

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x8000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        @instance.remove_break 0x8000, :program

        # Set a breakpoint at the start and end of the ISR.
        @instance.break int_breakpoint_start, :program
        @instance.break int_breakpoint_end, :program

        bench(1) do
            @instance.continue 10000
            isr_cycles = @instance.continue 10000
            
            isr_cycles
        end
    end

    def benchmark_interrupt_rx
        # Get symbols.
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_breakpoint_start = kernel_symbols.find_by_name("_interrupt_handler").address
        int_breakpoint_end = kernel_symbols.find_by_name("__interrupt_handler_end").address

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x8000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        @instance.remove_break 0x8000, :program

        # Set a breakpoint at the start and end of the ISR.
        @instance.break int_breakpoint_start, :program
        @instance.break int_breakpoint_end, :program

        bench(5) do
            # Write a character to the serial port.
            @instance.serial_puts "A"

            # Run, we should hit the ISR.
            @instance.continue 10000

            # Run again until we exit the ISR.
            isr_cycles = @instance.continue 10000
            
            isr_cycles
        end
    end

    def benchmark_scheduler_single
        # Get symbols.
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_breakpoint_start = kernel_symbols.find_by_name("__timer_handler").address
        int_breakpoint_end = kernel_symbols.find_by_name("__timer_handler_end").address

        # We expect to start executing at 0x8000,
        # where the command-processor would reside normally.
        @instance.break 0x8000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        @instance.remove_break 0x8000, :program

        # Set a breakpoint at the start and end of the ISR.
        @instance.break int_breakpoint_start, :program
        @instance.break int_breakpoint_end, :program

        bench(5) do
            # Run, we should hit the ISR.
            @instance.continue 10000000

            # Run again until we exit the ISR.
            isr_cycles = @instance.continue 100000
            
            isr_cycles
        end
    end

    def benchmark_scheduler_two_tasks
        # Get symbols.
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_breakpoint_start = kernel_symbols.find_by_name("__timer_handler").address
        int_breakpoint_end = kernel_symbols.find_by_name("__timer_handler_end").address

        # We expect to start executing at 0x8000,
        # where the command-processor would reside normally.
        @instance.break 0x8000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        @instance.remove_break 0x8000, :program

        # Set a breakpoint at the start and end of the ISR.
        @instance.break int_breakpoint_start, :program
        @instance.break int_breakpoint_end, :program

        bench(10) do
            # Run, we should hit the ISR.
            @instance.continue 10000000

            # Run again until we exit the ISR.
            isr_cycles = @instance.continue 100000
            
            isr_cycles
        end
    end
end

def benchmarks
    b = InterruptBenchmarks.new
    b.benchmarks()
end

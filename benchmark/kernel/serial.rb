require_relative 'base'

class SerialBenchmarks < KernelBenchmark
    def benchmark_serial_write
        # Get symbols.
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_swrite_start = kernel_symbols.find_by_name("_do_swrite").address
        int_swrite_end = kernel_symbols.find_by_name("_syscall_common_ret").address

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        @instance.remove_break 0x6000, :program

        # Set a breakpoint at the start and end of the ISR.
        @instance.break int_swrite_start, :program
        @instance.break int_swrite_end, :program

        bench(1000) do
            @instance.continue 10000
            isr_cycles = @instance.continue 10000
            
            isr_cycles
        end
    end
end

def benchmarks
    b = SerialBenchmarks.new
    b.benchmarks()
end

require_relative 'base'

class SerialBenchmarks < KernelBenchmark
    def benchmark_serial_write
        # Get symbols.
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        swrite_start = kernel_symbols.find_by_name("_driver_6850_tx").address
        swrite_end = kernel_symbols.find_by_name("_driver_6850_tx_done").address

        puts "%04x, %04x" % [swrite_start, swrite_end]

        # We expect to start executing at 0x8000,
        # where the command-processor would reside normally.
        @instance.break 0x8000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        @instance.remove_break 0x8000, :program

        # Set a breakpoint at the start and end of the ISR.
        @instance.break swrite_start, :program
        @instance.break swrite_end, :program

        bench(1) do
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

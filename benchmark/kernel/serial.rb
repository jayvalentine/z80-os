require_relative 'base'

class SerialBenchmarks < KernelBenchmark
    def benchmark_serial_write
        # Get symbols.
        kernel_symbols = load_map()

        swrite_start = kernel_symbols.find_by_name("_driver_6850_tx").address
        swrite_end = kernel_symbols.find_by_name("_driver_6850_tx_done").address

        # Set a breakpoint at the start and end of the ISR.
        @instance.break swrite_start, :program
        @instance.break swrite_end, :program

        bench(40) do
            @instance.continue 100000
            isr_cycles = @instance.continue 10000

            # Read from serial port so that it's ready
            # for the next time round.
            @instance.serial_gets(1)

            isr_cycles
        end
    end
end

def benchmarks
    b = SerialBenchmarks.new
    b.benchmarks()
end

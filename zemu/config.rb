require_relative '../../zemu/lib/zemu'

class Displayable < Zemu::Config::BusDevice
    def initialize
        super

        @display = true
    end

    def display_on
        @display = true
    end
    
    def display_off
        @display = false
    end

    def log(msg)
        if @display
            puts msg
        end
    end
end

# Banked RAM
# Memory object.
class BankedMemory < Displayable
    # Constructor.
    def initialize
        @current_bank = 0

        # 2D array containing memory banks.
        @contents = []
        256.times do |b|
            @contents << []
        end

        super

        # For each bank, pad out to the right
        # number of bytes.
        banks.times do |b|
            bank_size = @contents[b].size
            (size - bank_size).times do
                @contents[b] << padding
            end
        end
    end

    def bank
        @current_bank
    end

    # Gets or sets an array of bytes representing the initial state
    # of this memory block.
    def contents(bank, *args)
        if args.size == 0
            @contents[bank]
        else
            @contents[bank] = args[0].clone()
        end
    end

    # Is this memory read-only?
    def readonly?
        false
    end

    # Memory bus read handler.
    #
    # Handles read access via the memory bus to this device.
    #
    # @param addr The address being accessed.
    #
    # Returns the value read, or nil if no value
    # (e.g. if address falls outside range for this device).
    def mem_read(addr)
        # Return value in memory's contents if the address
        # falls within range.
        if (addr >= address) && (addr < (address + size))
            offset = addr - address
            value = @contents[@current_bank][offset]
            return value
        end

        # Otherwise return nil - address does not correspond
        # to this memory block.
        nil
    end

    # Memory bus write handler.
    #
    # Handles write access via the memory bus to this device.
    #
    # @param addr The address being accessed.
    # @param value The value being written.
    def mem_write(addr, value)
        # If address falls within range, set value in
        # memory contents.
        if (addr >= address) && (addr < (address + size))
            offset = addr - address
            @contents[@current_bank][offset] = value
        end
    end

    # IO write handler.
    def io_write(port, value)
        if port == bank_select
            @current_bank = value % banks
            log "Bank set to #{@current_bank}"
        end
    end

    # Valid parameters for this object.
    # Should be extended by subclasses but NOT REPLACED.
    def params
        super + %w(address size banks padding bank_select)
    end

    # Reads the contents of a file in binary format and
    # returns them as an array.
    def from_binary(file)
        return File.open(file, "rb") do |f|
            bin = []

            f.each_byte { |b| bin << b }

            bin
        end
    end
end

# Status LEDs object
#
# Represents the status LEDs on the z80 computer.
class StatusLedPort < Displayable
    def initialize
        super

        @register = 0
    end

    def io_write(port, value)
        if (port == io_port)
            @register = value

            return if !@display

            8.times do |i|
                s = "%-4s" % labels[i]
                print(s)
            end
            print("\n")

            8.times do |i|
                if (value & 0x80) == 0x80
                    print("0   ")
                else
                    print("1   ")
                end

                value = value << 1
            end
            print("\n\n")
        end
    end

    def register
        @register
    end

    def params
        super + %w(io_port labels)
    end
end

class Timer < Displayable
    def initialize
        super

        @clock_period = 1.0 / 3_686_400
        @count = period

        @interrupt = false
    end

    def params
        super + %w(io_port period)
    end

    def io_read(port)
        if port == io_port
            if @interrupt
                log "Timer reset"
                @interrupt = false
                @count = period
                return 1
            else
                return 0
            end
        end
    end

    def clock(cycles)
        if (@count > 0)
            @count -= (@clock_period * cycles)
        elsif !@interrupt && @count <= 0
            log "Timer trigger"
            @interrupt = true
        end

        interrupt(@interrupt)
    end
end

# Serial Input/Output object
#
# Represents a serial connection between the emulated CPU
# and the host machine, with input and output mapped to Z80 I/O
# ports.
class Serial6850 < Zemu::Config::BusDevice
    # Constructor.
    #
    # Takes a block in which the parameters of the serial port
    # can be initialized.
    #
    # All parameters can be set within this block.
    # They become readonly as soon as the block completes.
    #
    # @example
    #   
    #   Zemu::Config::SerialPort.new do
    #       name "serial"
    #       in_port 0x00
    #       out_port 0x01
    #   end
    #
    #
    def initialize
        super

        @buffer_rx = []
        @buffer_tx = []

        @status = 0
        @control = 0
        @int_rx = false
    end

    def transmitted_count
        @buffer_tx.size
    end

    def get_byte()
        @status |= 0x02
        return @buffer_tx.shift()
    end

    def put_byte(b)
        @buffer_rx = [b]
        @status |= 0x01
        @int_rx = true
    end

    def io_read(port)
        if port == data_port
            @status &= ~0x01
            @int_rx = false
            return @buffer_rx.shift()
        elsif port == control_port
            return @status
        end

        nil
    end

    def io_write(port, value)
        if port == data_port
            @status &= ~0x02
            @buffer_tx = [value]
        elsif port == control_port
            if ((value & 0x03) == 0x03)
                @status = 0x02
            else
                @control = value
            end
        end
    end

    def clock(cycles)
        if ((@control & 0x20) == 0x20) && ((@status & 0x02) == 0x02)
            int_tx = true
        else
            int_tx = false
        end

        if @int_rx || int_tx
            @status |= 0x80
            interrupt(true)
        else
            @status &= ~0x80
            interrupt(false)
        end
    end

    # Valid parameters for a Serial6850, along with those
    # defined in [Zemu::Config::BusDevice].
    def params
        super + %w(data_port control_port)
    end
end

def pad(array, size, value)
    if array.size >= size
        array
    else
        array + Array.new(size - array.size, value)
    end
end

def zemu_config(instance_name, binary, disk)
    conf = Zemu::Config.new do
        name instance_name

        clock_speed 3_686_400
        serial_delay 0.001

        add_memory (Zemu::Config::RAM.new do
            name "ram_kernel"
            address 0x0000
            size    0x8000
            
            # Pad with halts so we detect out-of-bounds accesses.
            contents pad(from_binary("kernel_debug.bin"), 0x8000, 0x76)
        end)

        add_memory (BankedMemory.new do
            name "banked_ram"
            address 0x8000
            size    0x8000

            banks 16
            bank_select 0x30

            padding 0x76

            contents 0, pad(from_binary(binary), 0x8000, 0x76)
            contents(0)[0x7fff] = 0xf7
            contents(0)[0x7ffe] = 0xec
            
            contents(0)[0x77ff] = 0x00
            contents(0)[0x77fe] = 0x00
            contents(0)[0x77fd] = 0x00
            contents(0)[0x77fc] = 0x00

            contents(0)[0x77fb] = 0x00
            contents(0)[0x77fa] = 0x00

            contents(0)[0x77f9] = 0x80
            contents(0)[0x77f8] = 0x00
        end)

        add_device (Timer.new do
            name "timer"
            io_port 0x10
            period 0.02
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

            initialize_from disk
        end)

        add_io (StatusLedPort.new do
            name "status"
            io_port 0x80
            labels %w(X X X X X SYS INT MEM)
        end)
    end

    conf
end

def zemu_start(binary_file="command.bin")
    config = zemu_config("debug", binary_file, "disk_copy.bin")

    Zemu.start_interactive(config, print_serial: false)
end

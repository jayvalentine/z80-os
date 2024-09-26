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
    attr_reader :bank_history
    
    # Constructor.
    def initialize
        @current_bank = 0
        @bank_history = []

        # 2D array containing memory banks.
        @contents = []
        256.times do |b|
            @contents << {}
        end

        super

        256.times do |b|
            @contents[b].default = @padding
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
            h = {}
            args[0].each_with_index do |v, i|
                h[i] = v
            end

            @contents[bank] = h
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

            raise "Value in banked memory should not be nil." if value.nil?
            
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
            #puts "Write %04x: %02x" % [addr, value]
            offset = addr - address
            @contents[@current_bank][offset] = value
        end
    end

    # IO write handler.
    def io_write(port, value)
        if port == bank_select
            @current_bank = value % banks
            @bank_history << @current_bank
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

# 8254 Timer Simulation
class Timer8254 < Displayable
    # An individual timer in the 8254.
    class Timer
        MODE_UNDEFINED = -1
        MODE_0 = 0

        attr_reader :out

        def initialize(device, id)
            @device = device
            @id = id

            reset(true)
        end

        def mode=(m)
            log "mode set: #{m}"
            @mode = m
        end

        def log(msg)
            @device.log "Timer #{@id}: #{msg}"
        end

        def reset(initial_reset=false)
            log "reset" unless initial_reset

            @mode = MODE_UNDEFINED
            @count = nil
            @next_count = nil

            @lsb = nil

            @out = 0
        end

        # Loads a byte into the counter.
        # Counter value is loaded LSB-first.
        def load_byte(val)
            @out = 0
            
            if @lsb.nil?
                log "Load lsb: 0x%02x" % val

                @lsb = val
            else
                log "Load msb: 0x%02x" % val

                msb = val
                initial_count = (msb << 8) | @lsb
                @next_count = initial_count

                @lsb = nil
            end
        end

        def tick
            return if @mode == MODE_UNDEFINED
            
            # Mode 0.
            # Decrements the timer if no count has been written.
            # Otherwise, loads the count into the timer.
            if @next_count.nil?
                if @count == 0
                    @out = 1
                elsif !@count.nil?
                    @count -= 1
                end
            else
                log "load new count: #{@next_count}"
                @count = @next_count
                @next_count = nil
            end
        end
    end



    # Constructor
    #
    # Takes a block used to set parameters.
    # The following parameters can be set:
    #
    # * base_port: Base port of the device.
    def initialize
        super

        @timers = []
        3.times do |i|
            @timers << Timer.new(self, i)
        end
    end

    def params
        super + %w(base_port)
    end

    def addressed?(port)
        port >= base_port && port < (base_port + 4)
    end

    def write_control(value)
        bcd = value & 0b0000_0001
        m = (value & 0b0000_1110) >> 1
        rw = (value & 0b0011_0000) >> 4
        sc = (value & 0b1100_0000) >> 6

        # Figure out which timer has been selected.
        raise "Read-back command not supported" if sc > 2
        timer = @timers[sc]

        # Figure out which mode has been selected.
        raise "Unsupported M val: #{m}" if m > 0
        selected_mode = case m
        when 0
            Timer::MODE_0
        end

        # Figure out BCD or binary mode.
        raise "BCD mode not supported" unless bcd == 0

        # Figure out r/w mode.
        # Only two-byte write mode supported currently.
        raise "Unsupported R/W mode: #{rw}" unless rw == 3

        # Reset the selected timer and select mode.
        timer.reset()
        timer.mode = selected_mode
    end

    def write_timer(addr, value)
        timer = @timers[addr]

        timer.load_byte(value)
    end

    def io_write(port, value)
        return unless addressed?(port)

        addr = port - base_port

        case addr
        when 0..2
            write_timer(addr, value)
        when 3
            write_control(value)
        end
    end

    def clock(cycles)
        cycles.times do
            @timers.each { |t| t.tick() }
        end

        # All timer OUTs are ORed
        # to INT.
        if @timers.any? { |t| t.out == 1 }
            interrupt(true)
        else
            interrupt(false)
        end
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

def zemu_config(instance_name, binary, disk, kernel_bin="kernel.bin")
    conf = Zemu::Config.new do
        name instance_name

        clock_speed 3_686_400
        serial_delay 0.001

        add_memory (Zemu::Config::RAM.new do
            name "ram_kernel"
            address 0x0000
            size    0x8000
            
            # Pad with halts so we detect out-of-bounds accesses.
            contents pad(from_binary(kernel_bin), 0x8000, 0x76)
        end)

        add_memory (BankedMemory.new do
            name "banked_ram"
            address 0x8000
            size    0x8000

            banks 16
            bank_select 0x30

            padding 0x76

            contents 0, from_binary(binary)
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

        add_device (Timer8254.new do
            name "timer"
            base_port 0x10
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

def zemu_start(kernel="kernel.bin", binary_file="command.bin", disk="disk_copy.bin")
    config = zemu_config("debug", binary_file, disk, kernel)

    Zemu.start_interactive(config, print_serial: false)
end

require 'minitest/autorun'
require_relative 'base'

# Tests for serial input/output.
class SIOTest < IntegrationTest
    # Tests that calling the swrite syscall writes characters to the serial output.
    def test_swrite
        compile_test_code(["kernel/integration_test/test_swrite.c"], "test_swrite.bin")

        start_instance("test_swrite.bin")

        expected = "hello, world!";
        
        # Get output string.
        output_string = ""
        expected.size.times do
            # Then continue for 2000 cycles.
            @instance.continue 2000
        
            # Should not have halted or hit breakpoint.
            assert !@instance.halted?, "Program halted unexpectedly."
            assert !@instance.break?, "Program hit breakpoint unexpectedly."

            output_string += @instance.serial_gets(1)
        end

        assert_equal expected, output_string, "Did not get expected serial output"
    end

    # Tests that calling the swrite syscall does not result in overflow
    # in the character buffer.
    def test_swrite_no_overflow
        compile_test_code(["kernel/integration_test/test_swrite_no_overflow.c"], "test_swrite_no_overflow.bin")

        start_instance("test_swrite_no_overflow.bin")

        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")
        swrite_breakpoint = kernel_symbols.find_by_name("__swrite_is_available").address

        @instance.break swrite_breakpoint, :program

        count = 0

        100000.times do
            # Then continue until we hit breakpoint.
            @instance.continue
        
            assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
            assert_equal swrite_breakpoint, @instance.registers["PC"], "Breakpoint at wrong address."
        
            # Check value of HL.
            hl = @instance.registers["HL"]
            assert (hl >= 0x100), "Head underflow: %04x" % hl
            assert (hl < 0x200), "Head overflow: %04x" % hl

            # Check no writes outside of the buffer.
            assert_equal 0xf3, @instance.memory(0x200)

            if count == 100
                @instance.serial_gets(1)
                count = 0
            end

            count += 1
        end
    end

    # Tests that calling the sread syscall reads characters from the serial output.
    def test_sread
        compile_test_code(["kernel/integration_test/test_sread.c"], "test_sread.bin")

        start_instance("test_sread.bin")

        # Send serial string.
        input_string = "hello"
        
        # Get output string.
        input_string.size.times do |i|
            # Send character.
            @instance.serial_puts(input_string[i])

            # Then continue for 5000 cycles.
            @instance.continue 5000
        end

        # Should have halted.
        assert @instance.halted?, "Program should have halted."

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end

    # Tests that the break command is received correctly.
    def test_sbreak
        compile_test_code(["kernel/integration_test/test_sbreak.c"], "test_sbreak.bin")

        start_instance("test_sbreak.bin")

        # Run for a little bit - we expect not to halt.
        @instance.continue 10000
        assert !@instance.halted?, "Program halted unexpectedly."

        # Send break character - 0x18.
        @instance.serial_puts(0x18.chr)

        # Now run and expect to halt.
        @instance.continue 10000
        assert @instance.halted?, "Program did not halt when expected."

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end

    # Tests that the break command is interpreted as a byte when serial port
    # is in binary mode.
    def test_smode_binmode
        compile_test_code(["kernel/integration_test/test_smode_binmode.c"], "test_smode_binmode.bin")

        start_instance("test_smode_binmode.bin")

        # Run for a little bit - we expect not to halt.
        @instance.continue 10000
        assert !@instance.halted?, "Program halted unexpectedly."

        # Send break character - 0x18.
        @instance.serial_puts(0x18.chr)

        # Now run and expect to halt.
        @instance.continue 10000
        assert @instance.halted?, "Program did not halt when expected."

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end
end

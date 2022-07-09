require 'minitest/autorun'
require_relative 'base'

# Tests for serial input/output.
class SIOTest < IntegrationTest
    # Tests that calling the swrite syscall writes characters to the serial output.
    def test_swrite
        compile_test_code(["kernel/integration_test/test_swrite.c"], "test_swrite.bin")

        start_instance("test_swrite.bin")

        output_string = ""

        12.times do
            # Get output character.
            @instance.continue 2000
        
            # Should not have halted or hit breakpoint.
            assert !@instance.halted?, "Program halted unexpectedly"
            assert !@instance.break?, "Program hit break unexpectedly"

            output_string += @instance.serial_gets(1)
        end

        # Final run, should hit halt.
        @instance.continue 2000
        output_string += @instance.serial_gets(1)
        @instance.continue 2000
        
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]

        assert_equal "hello, world!", output_string, "Did not get expected serial output"
    end

    # Tests that calling the swrite syscall can write a single character to the serial output.
    def test_swrite_one
        compile_test_code(["kernel/integration_test/test_swrite_one.c"], "test_swrite_one.bin")

        start_instance("test_swrite_one.bin")

        @instance.continue 2000
        
        # Should have halted.
        assert @instance.halted?, "Program should have halted."

        output_string = @instance.serial_gets(1)

        assert_equal "F", output_string, "Did not get expected serial output"
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

    # Tests that calling the sread syscall reads characters from the serial output.
    def test_sread_multiple
        compile_test_code(["kernel/integration_test/test_sread_gets.c"], "test_sread_gets.bin")

        start_instance("test_sread_gets.bin")

        # Send serial string.
        input_string = "hello\n"
        
        # Get output string.
        input_string.size.times do |i|
            # Send character.
            @instance.serial_puts(input_string[i])

            # Then continue for 10000 cycles.
            @instance.continue 5000

            # Receive character to free up tx buffer for next iteration.
            @instance.serial_gets(1)
        end

        @instance.continue 5000

        assert_program_finished

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
        assert @instance.halted?, "Program did not halt when expected (at address %04x)" % @instance.registers["PC"]

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

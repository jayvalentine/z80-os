require 'minitest/autorun'
require_relative 'base'
require_relative 'test_helper'

# Tests for serial input/output.
class SIOTest < IntegrationTest
    # Tests that calling the swrite syscall writes characters to the serial output.
    def test_swrite
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
        @instance.continue 1000
        
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]

        assert_equal "hello, world!", output_string, "Did not get expected serial output"
    end

    # Tests that calling the swrite syscall can write a single character to the serial output.
    def test_swrite_one
        @instance.continue 1000
        
        # Should have halted.
        assert @instance.halted?, "Program should have halted."

        output_string = @instance.serial_gets(1)

        assert_equal "F", output_string, "Did not get expected serial output"
    end

    # Tests that calling the sread syscall reads characters from the serial output.
    def test_sread
        # Send serial string.
        input_string = "hello"
        
        # Get output string.
        input_string.size.times do |i|
            # Send character.
            @instance.serial_puts(input_string[i])

            # Then continue for 2000 cycles.
            @instance.continue 2000
        end

        # Should have halted.
        assert @instance.halted?, "Program should have halted."

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end

    # Tests that calling the sread syscall reads characters from the serial output.
    def test_sread_multiple
        # Send serial string.
        input_string = "hello\n"
        
        # Get output string.
        input_string.size.times do |i|
            # Send character.
            @instance.serial_puts(input_string[i])

            # Then continue for 3000 cycles.
            @instance.continue 3000

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
        # Run for a little bit - we expect not to halt.
        @instance.continue 2000
        assert !@instance.halted?, "Program halted unexpectedly (at address %04x)." % @instance.registers["PC"]

        # Send break character - 0x18.
        @instance.serial_puts(0x18.chr)

        # Now run and expect to halt on the next scheduler tick.
        @instance.continue 200000
        assert @instance.halted?, "Program did not halt when expected (at address %04x)" % @instance.registers["PC"]

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end

    # Tests that the break command is received correctly while waiting for input
    # from the serial terminal.
    def test_sbreak_waiting_for_input
        # Run for a little bit - we expect not to halt.
        @instance.continue 2000
        assert !@instance.halted?, "Program halted unexpectedly (at address %04x)." % @instance.registers["PC"]

        # Send break character - 0x18.
        @instance.serial_puts(0x18.chr)

        # Now run and expect to halt on the next scheduler tick.
        @instance.continue 200000
        assert @instance.halted?, "Program did not halt when expected (at address %04x)" % @instance.registers["PC"]

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end

    # Tests that the break command is received by the current interactive process.
    def test_sbreak_multiprocess
        # Compile and load the code for the child process.
        #
        # This is at a different address to the parent so that
        # we can be sure the correct SIG_CANCEL handler is being called
        # in each case.
        compile_user_code(0xc000)
        load_user_program(0xc000)

        # Run for some time - we expect not to halt.
        # It takes a while to write the child process to disk.
        assert_running(cycles: 10000000)

        # Send a character to the serial terminal.
        # This should be received by the child process.
        @instance.serial_puts('A')
        assert_running(cycles: 200000)

        # Send break character - 0x18.
        @instance.serial_puts(0x18.chr)

        # Now run and expect not to halt.
        # We should have returned to the parent process.
        assert_running(cycles: 200000)

        # Send a character to the serial terminal.
        # This should be received by the parent process.
        @instance.serial_puts('B')
        assert_running(cycles: 200000)

        # Send another break character.
        @instance.serial_puts(0x18.chr)

        # Continue - program should continue until completion.
        @instance.continue 200000
        assert_program_finished

        # Check return value.
        assert_return_equal(0)
    end

    # Tests that the break command kills a process if no handler is registered.
    def test_sbreak_kills_by_default
        # Compile and load the code for the child process.
        #
        # This is at a different address to the parent so that
        # we can be sure the correct SIG_CANCEL handler is being called
        # in each case.
        compile_user_code(0xc000)
        load_user_program(0xc000)

        # Run for some time - we expect not to halt.
        # It takes a while to write the child process to disk.
        assert_running(cycles: 10000000)

        # Send break character - 0x18.
        # The child process should exit.
        @instance.serial_puts(0x18.chr)

        # Now run and expect not to halt.
        # We should have returned to the parent process.
        assert_running(cycles: 200000)

        # Send a character to the serial terminal.
        # This should be received by the parent process.
        @instance.serial_puts('C')
        assert_running(cycles: 200000)

        # Send another break character.
        @instance.serial_puts(0x18.chr)

        # Continue - program should continue until completion.
        @instance.continue 200000
        assert_program_finished

        # Check return value.
        assert_return_equal(0)
    end

    # Tests that the break command is interpreted as a byte when serial port
    # is in binary mode.
    def test_smode_binmode
        # Run for a little bit - we expect not to halt.
        @instance.continue 2000
        assert !@instance.halted?, "Program halted unexpectedly."

        # Send break character - 0x18.
        @instance.serial_puts(0x18.chr)

        # Now run and expect to halt.
        @instance.continue 2000
        assert @instance.halted?, "Program did not halt when expected."

        # Check return value.
        assert_equal 0x0000, @instance.registers["HL"], "Wrong return value!"
    end
end

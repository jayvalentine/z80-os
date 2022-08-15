require 'minitest/autorun'
require_relative 'base'
require_relative 'test_helper'

class StatusTest < IntegrationTest
    # Tests that the correct status bit gets set
    # when entering the ISR, and unset when leaving.
    def test_status_int
        # Get symbols.
        prog_symbols = load_test_map()
        kernel_symbols = load_kernel_map()

        int_breakpoint = kernel_symbols.find_by_name("__serial_read_handler")
        assert !int_breakpoint.nil?, "Could not find symbol for serial read handler!"

        prog_breakpoint = prog_symbols.find_by_name("_test_func")
        assert !prog_breakpoint.nil?, "Could not find symbol for test function!"

        # Interrupt status bit should be unset.
        status = @instance.device("status").register
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Set a breakpoint for the test function.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue 1000000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should still be unset.
        status = @instance.device("status").register
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Remove the program breakpoint, set a breakpoint in the ISR.
        @instance.remove_break prog_breakpoint.address, :program
        @instance.break int_breakpoint.address, :program

        # Send something over the serial port to trigger the ISR.
        @instance.serial_puts "H"

        # Continue, we should hit the ISR breakpoint.
        @instance.continue 1000000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal int_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should now be set.
        status = @instance.device("status").register
        assert_equal 0b11111101, status, "Status LED not set!"

        # Set breakpoint on program.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue 1000000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should now be unset.
        status = @instance.device("status").register
        assert_equal 0b11111111, status, "Status LED still set after exiting ISR!"
    end

    # Tests that the correct status bit gets set
    # when entering a syscall, and unset when leaving.
    def test_status_syscall
        # Get symbols.
        prog_symbols = load_test_map()
        kernel_symbols = load_kernel_map()

        int_breakpoint = kernel_symbols.find_by_name("_driver_6850_tx")
        assert !int_breakpoint.nil?, "Could not find symbol for swrite handler!"

        prog_breakpoint = prog_symbols.find_by_name("_test_func")
        assert !prog_breakpoint.nil?, "Could not find symbol for test function!"

        # Syscall status bit should be unset.
        status = @instance.device("status").register
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Set a breakpoint for the test function.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue 1000000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should still be unset.
        status = @instance.device("status").register
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Remove the program breakpoint, set a breakpoint in the syscall routine.
        @instance.remove_break prog_breakpoint.address, :program
        @instance.break int_breakpoint.address, :program

        # Continue, we should hit the syscall breakpoint.
        @instance.continue 1000000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal int_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should now be set.
        status = @instance.device("status").register
        assert_equal 0b11111011, status, "Status LED not set!"

        # Set breakpoint on program.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue 1000000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should now be unset.
        status = @instance.device("status").register
        assert_equal 0b11111111, status, "Status LED still set after exiting ISR!"
    end
end

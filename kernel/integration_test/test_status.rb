require 'minitest/autorun'
require_relative 'base'

class StatusTest < IntegrationTest
    # Tests that the correct status bit gets set
    # when entering the ISR, and unset when leaving.
    def test_status_int
        compile_test_code(["kernel/integration_test/test_status_int.c"], "test_status_int.bin")

        # Get symbols.
        prog_symbols = Zemu::Debug.load_map("test_status_int.map")
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_breakpoint = kernel_symbols.find_by_name("__serial_read_handler")
        assert !int_breakpoint.nil?, "Could not find symbol for serial read handler!"

        prog_breakpoint = prog_symbols.find_by_name("_test_func")
        assert !prog_breakpoint.nil?, "Could not find symbol for test function!"

        start_instance("test_status_int.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should be unset.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Set a breakpoint for the test function.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should still be unset.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Remove the program breakpoint, set a breakpoint in the ISR.
        @instance.remove_break prog_breakpoint.address, :program
        @instance.break int_breakpoint.address, :program

        # Send something over the serial port to trigger the ISR.
        @instance.serial_puts "H"

        # Continue, we should hit the ISR breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal int_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should now be set.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111101, status, "Status LED not set!"

        # Set breakpoint on program.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Interrupt status bit should now be unset.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111111, status, "Status LED still set after exiting ISR!"
    end

    # Tests that the correct status bit gets set
    # when entering a syscall, and unset when leaving.
    def test_status_syscall
        compile_test_code(["kernel/integration_test/test_status_syscall.c"], "test_status_syscall.bin")

        # Get symbols.
        prog_symbols = Zemu::Debug.load_map("test_status_syscall.map")
        kernel_symbols = Zemu::Debug.load_map("kernel_debug.map")

        int_breakpoint = kernel_symbols.find_by_name("_do_swrite")
        assert !int_breakpoint.nil?, "Could not find symbol for swrite handler!"

        prog_breakpoint = prog_symbols.find_by_name("_test_func")
        assert !prog_breakpoint.nil?, "Could not find symbol for test function!"

        start_instance("test_status_syscall.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should be unset.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Set a breakpoint for the test function.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should still be unset.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111111, status, "Status LED unexpectedly set!"

        # Remove the program breakpoint, set a breakpoint in the syscall routine.
        @instance.remove_break prog_breakpoint.address, :program
        @instance.break int_breakpoint.address, :program

        # Continue, we should hit the syscall breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal int_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should now be set.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111011, status, "Status LED not set!"

        # Set breakpoint on program.
        @instance.break prog_breakpoint.address, :program
        
        # Continue until we hit the breakpoint.
        @instance.continue
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal prog_breakpoint.address, @instance.registers["PC"], "Breakpoint at wrong address."

        # Syscall status bit should now be unset.
        status = @instance.zemu_io_status_value
        assert_equal 0b11111111, status, "Status LED still set after exiting ISR!"
    end
end

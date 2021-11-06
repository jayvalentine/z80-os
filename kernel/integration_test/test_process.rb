require 'minitest/autorun'
require_relative 'base'

class ProcessTest < IntegrationTest
    # Tests:
    # * That calling pexec syscall executes code in user memory.
    def test_pexec_simple
        compile_test_code(["kernel/integration_test/test_pexec_simple.c"], "test_pexec_simple.bin")

        start_instance("test_pexec_simple.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0x8000.
        @instance.break 0x8000, :program

        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x8000, @instance.registers["PC"], "Breakpoint at wrong address."
    end

    # Tests:
    # * That calling pexec syscall executes code in user memory.
    # * Tests with a different address to the "normal" user code address
    #   to ensure we can call user code at different addresses.
    def test_pexec_different_address
        compile_test_code(["kernel/integration_test/test_pexec_different_address.c"], "test_pexec_different_address.bin")

        start_instance("test_pexec_different_address.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0xd000.
        @instance.break 0xd000, :program

        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xd000, @instance.registers["PC"], "Breakpoint at wrong address."
    end

    # Tests:
    # * That calling pexec syscall executes code in user memory.
    # * Tests with a different address to the "normal" user code address
    #   to ensure we can call user code at different addresses.
    # * That arguments are passed correctly.
    def test_pexec_different_address_with_args
        compile_test_code(["kernel/integration_test/test_pexec_different_address_with_args.c"], "test_pexec_different_address_with_args.bin")
        compile_user_code(0xd000, ["kernel/integration_test/test_pexec_different_address_with_args_user.c"], "test_pexec_different_address_with_args_user.bin")

        start_instance("test_pexec_different_address_with_args.bin")
        load_user_program(0xd000, "test_pexec_different_address_with_args_user.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0xd000.
        @instance.break 0xd000, :program

        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xd000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt. User program should return 0.
        # Test program will return whatever the user program returns.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
    end

    # Tests:
    # * That passing arguments with the pexec syscall are passed correctly.
    def test_pexec_args
        compile_test_code(["kernel/integration_test/test_pexec_args.c"], "test_pexec_args.bin")
        compile_user_code(0x8000, ["kernel/integration_test/test_pexec_args_user.c"], "test_pexec_args_user.bin")

        start_instance("test_pexec_args.bin")
        load_user_program(0x8000, "test_pexec_args_user.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0x8000.
        @instance.break 0x8000, :program

        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x8000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt. User program should return 0.
        # Test program will return whatever the user program returns.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
    end

    def test_pload_simple
        compile_test_code(["kernel/integration_test/test_pload_simple.c"], "test_pload_simple.bin")

        start_instance("test_pload_simple.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt.
        # By this point the file should have been loaded at 0x8000.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
        
        # Assert contents of memory.
        assert_equal 0xa5, @instance.memory(0x8000)
        assert_equal 0xb6, @instance.memory(0x8001)
        assert_equal 0xc7, @instance.memory(0x8002)
        assert_equal 0xd8, @instance.memory(0x8003)
        assert_equal 0xe9, @instance.memory(0x8004)
    end

    def test_pload_different_address
        compile_test_code(["kernel/integration_test/test_pload_different_address.c"], "test_pload_different_address.bin")

        start_instance("test_pload_different_address.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt.
        # By this point the file should have been loaded at 0xd000.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
        
        # Assert contents of memory.
        assert_equal 0x76, @instance.memory(0x8000)
        assert_equal 0x76, @instance.memory(0x8001)
        assert_equal 0x76, @instance.memory(0x8002)
        assert_equal 0x76, @instance.memory(0x8003)
        assert_equal 0x76, @instance.memory(0x8004)

        assert_equal 0x11, @instance.memory(0xd000)
        assert_equal 0x22, @instance.memory(0xd001)
        assert_equal 0x33, @instance.memory(0xd002)
        assert_equal 0x44, @instance.memory(0xd003)
        assert_equal 0x55, @instance.memory(0xd004)
    end

    def test_pload_invalid_addr
        compile_test_code(["kernel/integration_test/test_pload_invalid_addr.c"], "test_pload_invalid_addr.bin")

        start_instance("test_pload_invalid_addr.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt.
        # File should not have been loaded and the test code should return E_INVALIDPAGE.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xfff6, @instance.registers["HL"] # -10 (E_INVALIDPAGE)
        
        # Assert contents of memory.
        assert 0x12 != @instance.memory(0x5000)
        assert 0x34 != @instance.memory(0x5001)
        assert 0x56 != @instance.memory(0x5002)
        assert 0x78 != @instance.memory(0x5003)
        assert 0x9a != @instance.memory(0x5004)
    end

    def test_pload_invalid_header
        compile_test_code(["kernel/integration_test/test_pload_invalid_header.c"], "test_pload_invalid_header.bin")

        start_instance("test_pload_invalid_header.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt.
        # File should not have been loaded and the test code should return E_INVALIDHEADER.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xfff5, @instance.registers["HL"] # -11 (E_INVALIDHEADER)
        
        # Assert contents of memory.
        assert 0x12 != @instance.memory(0x8000)
        assert 0x34 != @instance.memory(0x8001)
        assert 0x56 != @instance.memory(0x8002)
        assert 0x78 != @instance.memory(0x8003)
        assert 0x9a != @instance.memory(0x8004)
    end

    def test_pload_wrong_size_header
        compile_test_code(["kernel/integration_test/test_pload_wrong_size_header.c"], "test_pload_wrong_size_header.bin")

        start_instance("test_pload_wrong_size_header.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Then continue until halt.
        # File should not have been loaded and the test code should return E_INVALIDHEADER.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xfff5, @instance.registers["HL"] # -11 (E_INVALIDHEADER)
    end
end

require 'minitest/autorun'
require_relative 'base'

class ProcessTest < IntegrationTest
    # Tests:
    # * That calling pexec syscall executes a new process in user memory.
    def test_pexec_simple
        compile_test_code(["kernel/integration_test/test_pexec_simple.c"], "test_pexec_simple.bin")

        start_instance("test_pexec_simple.bin")

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0x8000, bank 1.
        @instance.break 0x8000, :program
        
        @instance.continue 1000000
        
        assert @instance.break?, "Did not hit breakpoint (at address %04x, bank %d)" % [@instance.registers["PC"], @instance.device("banked_ram").bank]
        assert_equal 0x8000, @instance.registers["PC"], "Breakpoint at wrong address."
        assert_equal 1, @instance.device("banked_ram").bank
    end

    # Tests:
    # * That calling pexec syscall executes code in user memory.
    # * Tests with a different address to the "normal" user code address
    #   to ensure we can call user code at different addresses.
    def test_pexec_different_address
        compile_test_code(["kernel/integration_test/test_pexec_different_address.c"], "test_pexec_different_address.bin")

        start_instance("test_pexec_different_address.bin")

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0xd000, bank 1
        @instance.break 0xd000, :program

        @instance.continue 1000000

        assert @instance.break?, "Did not hit breakpoint (at address %04x, bank %d)" % [@instance.registers["PC"], @instance.device("banked_ram").bank]
        assert_equal 0xd000, @instance.registers["PC"], "Breakpoint at wrong address."
        assert_equal 1, @instance.device("banked_ram").bank
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
        load_user_program(0xc000, "test_pexec_different_address_with_args_user.bin")
        
        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0xd000, bank 1.
        @instance.break 0xd000, :program

        @instance.continue 10000000

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xd000, @instance.registers["PC"], "Breakpoint at wrong address."
        assert_equal 1, @instance.device("banked_ram").bank

        # Then continue until halt. User program should return 0.
        # Test program will return whatever the user program returns.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x8008, @instance.registers["PC"], "Halted at wrong address (%04x, bank %d)" % [@instance.registers["PC"], @instance.device("banked_ram").bank]
        assert_equal 0x0000, @instance.registers["HL"], "wrong return value"
    end

    # Tests:
    # * That passing arguments with the pexec syscall are passed correctly.
    def test_pexec_args
        compile_test_code(["kernel/integration_test/test_pexec_args.c"], "test_pexec_args.bin")
        compile_user_code(0x8000, ["kernel/integration_test/test_pexec_args_user.c"], "test_pexec_args_user.bin")

        start_instance("test_pexec_args.bin")
        load_user_program(0xc000, "test_pexec_args_user.bin")

        # Then continue. The test code executes the pexec system call
        # so we should hit a breakpoint at 0x8000, bank 1.
        @instance.break 0x8000, :program

        @instance.continue 10000000

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x8000, @instance.registers["PC"], "Breakpoint at wrong address."
        assert_equal 1, @instance.device("banked_ram").bank

        # Then continue until halt. User program should return 0.
        # Test program will return whatever the user program returns.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x8008, @instance.registers["PC"], "Halted at wrong address (%04x, bank %d)" % [@instance.registers["PC"], @instance.device("banked_ram").bank]
        assert_equal 0x0000, @instance.registers["HL"], "wrong return value"
    end

    def test_pload_simple
        compile_test_code(["kernel/integration_test/test_pload_simple.c"], "test_pload_simple.bin")

        start_instance("test_pload_simple.bin")

        # Then continue until halt.
        # By this point the file should have been loaded at 0x8000.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
        
        # Assert contents of memory.
        assert_equal 0xa5, @instance.device("banked_ram").contents(1)[0x0000]
        assert_equal 0xb6, @instance.device("banked_ram").contents(1)[0x0001]
        assert_equal 0xc7, @instance.device("banked_ram").contents(1)[0x0002]
        assert_equal 0xd8, @instance.device("banked_ram").contents(1)[0x0003]
        assert_equal 0xe9, @instance.device("banked_ram").contents(1)[0x0004]
    end

    def test_pload_large
        compile_test_code(["kernel/integration_test/test_pload_large.c"], "test_pload_large.bin")

        start_instance("test_pload_large.bin")

        # Then continue until halt.
        # By this point the file should have been loaded at 0x8000.
        @instance.continue 10000000

        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
        
        # Assert contents of memory.
        # First 2048 bytes should be 128-byte blocks of 10..25.
        base = 0
        (0...16).each do |i|
            128.times do |j|
                address = 0x8000 + base + j
                expected = i+10
                actual = @instance.device("banked_ram").contents(1)[base + j]
                assert_equal expected, actual, "Mismatch at address %04x (expected %d, got %d)" % [address, expected, actual]
            end
            base += 128
        end
    end

    def test_pload_different_address
        compile_test_code(["kernel/integration_test/test_pload_different_address.c"], "test_pload_different_address.bin")

        start_instance("test_pload_different_address.bin")

        # Then continue until halt.
        # By this point the file should have been loaded at 0xd000.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["HL"]
        
        # Assert contents of memory.
        assert_equal 0x76, @instance.device("banked_ram").contents(1)[0x0000]
        assert_equal 0x76, @instance.device("banked_ram").contents(1)[0x0001]
        assert_equal 0x76, @instance.device("banked_ram").contents(1)[0x0002]
        assert_equal 0x76, @instance.device("banked_ram").contents(1)[0x0003]
        assert_equal 0x76, @instance.device("banked_ram").contents(1)[0x0004]

        assert_equal 0x11, @instance.device("banked_ram").contents(1)[0x5000]
        assert_equal 0x22, @instance.device("banked_ram").contents(1)[0x5001]
        assert_equal 0x33, @instance.device("banked_ram").contents(1)[0x5002]
        assert_equal 0x44, @instance.device("banked_ram").contents(1)[0x5003]
        assert_equal 0x55, @instance.device("banked_ram").contents(1)[0x5004]
    end

    def test_pload_invalid_addr
        compile_test_code(["kernel/integration_test/test_pload_invalid_addr.c"], "test_pload_invalid_addr.bin")

        start_instance("test_pload_invalid_addr.bin")

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

        # Then continue until halt.
        # File should not have been loaded and the test code should return E_INVALIDHEADER.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xfff5, @instance.registers["HL"] # -11 (E_INVALIDHEADER)
        
        # Assert contents of memory.
        assert 0x12 != @instance.device("banked_ram").contents(1)[0x0000]
        assert 0x34 != @instance.device("banked_ram").contents(1)[0x0001]
        assert 0x56 != @instance.device("banked_ram").contents(1)[0x0002]
        assert 0x78 != @instance.device("banked_ram").contents(1)[0x0003]
        assert 0x9a != @instance.device("banked_ram").contents(1)[0x0004]
    end

    def test_pload_wrong_size_header
        compile_test_code(["kernel/integration_test/test_pload_wrong_size_header.c"], "test_pload_wrong_size_header.bin")

        start_instance("test_pload_wrong_size_header.bin")

        # Then continue until halt.
        # File should not have been loaded and the test code should return E_INVALIDHEADER.
        @instance.continue 1000000
        assert @instance.halted?, "Program did not halt (at address %04x)" % @instance.registers["PC"]
        assert_equal 0xfff5, @instance.registers["HL"] # -11 (E_INVALIDHEADER)
    end

    # Tests:
    # * That the pspawn syscall allows for running two processes concurrently.
    def test_pspawn_simple
        compile_test_code(["kernel/integration_test/test_pspawn_simple.c"], "test_pspawn_simple.bin")

        start_instance("test_pspawn_simple.bin")

        # Then continue. Both processes should run concurrently
        # so we should eventually see the expected values in memory.
        #
        # The spawned process runs forever so we are definitely
        # testing for concurrency.
        @instance.continue 500000
        assert !@instance.halted?, "Hit unexpected HALT (bank %d, addr %04x)" % [@instance.device("banked_ram").bank, @instance.registers["PC"]]
        assert !@instance.break?, "Hit unexpected BREAK"

        # Get symbols.
        prog_symbols = Zemu::Debug.load_map("test_pspawn_simple.map")
        val_addr = prog_symbols.find_by_name("_ret").address
        
        assert_equal 0, @instance.device("banked_ram").contents(0)[val_addr - 0x8000]

        assert_equal 99, @instance.device("banked_ram").contents(1)[0x4014]
        assert_equal 100, @instance.device("banked_ram").contents(1)[0x4015]
        assert_equal 101, @instance.device("banked_ram").contents(1)[0x4016]
        assert_equal 102, @instance.device("banked_ram").contents(1)[0x4017]
        assert_equal 103, @instance.device("banked_ram").contents(1)[0x4018]

        assert_equal 23, @instance.device("banked_ram").contents(0)[0x5123]
        assert_equal 34, @instance.device("banked_ram").contents(0)[0x5124]
        assert_equal 45, @instance.device("banked_ram").contents(0)[0x5125]
        assert_equal 56, @instance.device("banked_ram").contents(0)[0x5126]
        assert_equal 67, @instance.device("banked_ram").contents(0)[0x5127]
    end

    # Tests:
    # * That the pspawn syscall allows for spawning two processes concurrently.
    def test_pspawn_two_processes
        compile_test_code(["kernel/integration_test/test_pspawn_two_processes.c"], "test_pspawn_two_processes.bin")

        start_instance("test_pspawn_two_processes.bin")

        # Get symbols.
        prog_symbols = Zemu::Debug.load_map("test_pspawn_two_processes.map")
        val_addr = prog_symbols.find_by_name("_ret").address

        # Then continue. Both processes should run concurrently
        # so we should eventually see the expected values in memory.
        #
        # The spawned process runs forever so we are definitely
        # testing for concurrency.
        @instance.continue 1000000
        assert !@instance.halted?, "Hit unexpected HALT (bank %d, addr %04x, ret %d)" % [@instance.device("banked_ram").bank, @instance.registers["PC"], @instance.device("banked_ram").contents(0)[val_addr - 0x8000]]
        assert !@instance.break?, "Hit unexpected BREAK"

        assert_equal 0, @instance.device("banked_ram").contents(0)[val_addr - 0x8000]

        assert_equal 99, @instance.device("banked_ram").contents(1)[0x4014]
        assert_equal 100, @instance.device("banked_ram").contents(1)[0x4015]
        assert_equal 101, @instance.device("banked_ram").contents(1)[0x4016]
        assert_equal 102, @instance.device("banked_ram").contents(1)[0x4017]
        assert_equal 103, @instance.device("banked_ram").contents(1)[0x4018]

        assert_equal 99, @instance.device("banked_ram").contents(2)[0x4014]
        assert_equal 100, @instance.device("banked_ram").contents(2)[0x4015]
        assert_equal 101, @instance.device("banked_ram").contents(2)[0x4016]
        assert_equal 102, @instance.device("banked_ram").contents(2)[0x4017]
        assert_equal 103, @instance.device("banked_ram").contents(2)[0x4018]

        assert_equal 23, @instance.device("banked_ram").contents(0)[0x5123]
        assert_equal 34, @instance.device("banked_ram").contents(0)[0x5124]
        assert_equal 45, @instance.device("banked_ram").contents(0)[0x5125]
        assert_equal 56, @instance.device("banked_ram").contents(0)[0x5126]
        assert_equal 67, @instance.device("banked_ram").contents(0)[0x5127]
    end
end

require 'minitest/autorun'
require_relative 'base'
require_relative 'test_helper'

class BootTest < IntegrationTest
    def test_boot
        # Intentionally empty - we are just checking we can
        # start an instance and reach main().
    end

    def test_boot_invalid_syscall
        syms = Zemu::Debug.load_map("kernel_debug.map")
        sflags_addr = syms.find_by_name("_startup_flags").address

        # Startup flags should be 0.
        assert_equal 0x00, @instance.memory(sflags_addr), "wrong value in flags on startup"

        # We then expect to reset at 0x0008.
        # This is the cold start entry point.
        @instance.break 0x0008, :program

        # Continue until we hit the breakpoint.
        @instance.continue 1000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0008, @instance.registers["PC"], "Breakpoint at wrong address."

        # Startup flags should be set.
        assert_equal 0x01, @instance.memory(sflags_addr)
    end
end

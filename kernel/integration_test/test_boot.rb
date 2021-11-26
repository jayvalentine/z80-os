require 'minitest/autorun'
require_relative 'base'

class BootTest < IntegrationTest
    def test_boot
        File.open("test_boot.bin", "wb") do |f|
            f.write [0, 0, 0, 0].pack("CCCC")
        end

        start_instance("test_boot.bin")

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."
    end

    def test_boot_invalid_syscall
        compile_test_code(["kernel/integration_test/test_boot_invalid_syscall.asm"], "test_boot_invalid_syscall.bin")

        start_instance("test_boot_invalid_syscall.bin")

        syms = Zemu::Debug.load_map("kernel_debug.map")
        sflags_addr = syms.find_by_name("_startup_flags").address

        # Startup flags should be 0.
        assert_equal 0x00, @instance.memory(sflags_addr)

        # We expect to start executing at 0x6000,
        # where the command-processor would reside normally.
        @instance.break 0x6000, :program
        
        # Run, and expect to hit the breakpoint.
        @instance.continue

        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Startup flags should still be 0.
        assert_equal 0x00, @instance.memory(sflags_addr)

        # We then expect to reset at 0.
        @instance.break 0x0000, :program

        # Continue until we hit the breakpoint.
        @instance.continue 1000
        assert @instance.break?, "Did not hit breakpoint (at address %04x)" % @instance.registers["PC"]
        assert_equal 0x0000, @instance.registers["PC"], "Breakpoint at wrong address."

        # Startup flags should be set.
        assert_equal 0x01, @instance.memory(sflags_addr)
    end
end

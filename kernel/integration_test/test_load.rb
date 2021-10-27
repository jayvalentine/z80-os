require 'minitest/autorun'
require_relative 'base'

class LoadTest < IntegrationTest
    def test_load_simple
        compile_test_code(["kernel/integration_test/test_load_simple.c"], "test_load_simple.bin")

        start_instance("test_load_simple.bin")

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
end

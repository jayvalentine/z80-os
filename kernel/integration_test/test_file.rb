require 'minitest/autorun'
require_relative 'base'

# Tests for file handling.
class FileHandlingTest < IntegrationTest
    # Tests that calling the fentries syscall when there are no files on disk
    # returns zero.
    def test_fentries_empty
        compile_test_code(["kernel/integration_test/test_fentries_empty.c"], "test_fentries_empty.bin")

        start_instance("test_fentries_empty.bin")

        @instance.continue 100000

        # Should have halted at 0x8008.
        assert @instance.halted?, "Instance did not halt (at address 0x#{@instance.registers["PC"].to_s(16)})"
        assert_equal 0x8008, @instance.registers["PC"]

        # Main function should have returned 0, as that's the result
        # of the fentries call.
        assert_equal 0, @instance.registers["HL"]
    end

    # Tests that calling the fentries syscall when there are some files on disk
    # returns the number of files.
    def test_fentries_some_files
        compile_test_code(["kernel/integration_test/test_fentries_some_files.c"], "test_fentries_some_files.bin")

        start_instance("test_fentries_some_files.bin")

        @instance.continue 500000

        # Should have halted at 0x8008.
        assert @instance.halted?, "Instance did not halt (at address 0x#{@instance.registers["PC"].to_s(16)})"
        assert_equal 0x8008, @instance.registers["PC"]

        # Main function should have returned 3, as that's the result
        # of the fentries call.
        assert_equal 3, @instance.registers["HL"]
    end
end

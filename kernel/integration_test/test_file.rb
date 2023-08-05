require 'minitest/autorun'
require_relative 'base'
require_relative 'test_helper'

# Tests for file handling.
class FileHandlingTest < IntegrationTest
    # Tests that calling the fentries syscall when there are no files on disk
    # returns zero.
    def test_fentries_empty
        @instance.continue 100000

        assert_program_finished()

        # Main function should have returned 0, as that's the result
        # of the fentries call.
        assert_equal 0, @instance.registers["HL"]
    end

    # Tests that calling the fentries syscall when there are some files on disk
    # returns the number of files.
    def test_fentries_some_files
        @instance.continue 500000

        assert_program_finished()

        # Main function should have returned 3, as that's the result
        # of the fentries call.
        assert_equal 3, @instance.registers["HL"]
    end

    # Tests that the fentry syscall returns the right filename.
    def test_fentry
        @instance.continue 1000000

        assert_program_finished()

        # Main function should have returned 0.
        # Non-zero values are returned when the test code fails.
        assert_equal 0, @instance.registers["HL"]
    end
end

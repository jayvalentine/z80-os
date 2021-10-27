require 'minitest/autorun'
require_relative 'test_base'

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

    assert @instance.break?, "Did not hit breakpoint."
    assert_equal 0x6000, @instance.registers["PC"], "Breakpoint at wrong address."
end

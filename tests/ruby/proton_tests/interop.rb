#!/usr/bin/env ruby

require 'test/unit'
require 'qpid_proton'

if ((RUBY_VERSION.split(".").map {|x| x.to_i}  <=> [1, 9]) < 0)
  require 'pathname'
  class File
    def self.absolute_path(name)
      return Pathname.new(name).realpath
    end
  end
end

class InteropTest < Test::Unit::TestCase
  Data = Qpid::Proton::Data
  Message = Qpid::Proton::Message

  def setup
    @data = Data.new
    @message = Message.new
  end

  # Walk up the directory tree to find the tests directory.
  def get_data(name)
    path = File.absolute_path(__FILE__)
    while path and File.basename(path) != "tests" do path = File.dirname(path); end
    path = File.join(path,"interop")
    raise "Can't find test/interop directory from #{__FILE__}" unless File.directory?(path)
    path = File.join(path,"#{name}.amqp")
    File.open(path, "rb") { |f| f.read }
  end

  # Decode encoded bytes as a Data object
  def decode_data(encoded)
    buffer = encoded
    while buffer.size > 0
      n = @data.decode(buffer)
      buffer = buffer[n..-1]
    end
    @data.rewind
    reencoded = @data.encode
    # Test the round-trip re-encoding gives the same result.
    assert_equal(encoded, reencoded)
  end

  def decode_data_file(name) decode_data(get_data(name)); end

  def decode_message_file(name)
    message = Message.new()
    message.decode(self.get_data(name))
    self.decode_data(message.content)
  end

  def assert_next(type, value)
    assert @data.next
    assert_equal(type, @data.type)
    assert_equal(value, type.get(@data))
  end

  def test_message
    decode_message_file("message")
    assert_next(Data::STRING, "hello")
    assert !@data.next()
  end

  def test_primitives
    decode_data_file("primitives")
    assert_next(Data::BOOL, true)
    assert_next(Data::BOOL, false)
    assert_next(Data::UBYTE, 42)
    assert_next(Data::USHORT, 42)
    assert_next(Data::SHORT, -42)
    assert_next(Data::UINT, 12345)
    assert_next(Data::INT, -12345)
    assert_next(Data::ULONG, 12345)
    assert_next(Data::LONG, -12345)
    assert_next(Data::FLOAT, 0.125)
    assert_next(Data::DOUBLE, 0.125)
    assert !@data.next()
  end

  def test_strings
    decode_data_file("strings")
    assert_next(Data::BINARY, "abc\0defg")
    assert_next(Data::STRING, "abcdefg")
    assert_next(Data::SYMBOL, "abcdefg")
    assert_next(Data::BINARY, "")
    assert_next(Data::STRING, "")
    assert_next(Data::SYMBOL, "")
    assert !@data.next()
  end

  def test_described
    decode_data_file("described")
    assert_next(Data::DESCRIBED, Data::Described.new("foo-descriptor", "foo-value"))
    assert(@data.described?)
    assert_next(Data::DESCRIBED, Data::Described.new(12, 13))
    assert(@data.described?)
    assert !@data.next
  end

  def test_described_array
    decode_data_file("described_array")
    assert_next(Data::ARRAY, Data::Array.new("int-array", Data::INT, 0...10))
  end

  def test_arrays
    decode_data_file("arrays")
    assert_next(Data::ARRAY, Data::Array.new(false, Data::INT, 0...100))
    assert_next(Data::ARRAY, Data::Array.new(false, Data::STRING, ["a", "b", "c"]))
    assert_next(Data::ARRAY, Data::Array.new(false, Data::INT))
    assert !@data.next
  end

  def test_lists
    decode_data_file("lists")
    assert_next(Data::LIST, [32, "foo", true])
    assert_next(Data::LIST, [])
    assert !@data.next
  end

  def test_maps
    decode_data_file("maps")
    assert_next(Data::MAP, {"one" => 1, "two" => 2, "three" => 3 })
    assert_next(Data::MAP, {1 => "one", 2 => "two", 3 => "three"})
    assert_next(Data::MAP, {})
    assert !@data.next()
  end
end

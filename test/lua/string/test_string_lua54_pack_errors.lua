function test_string_byte_empty_count()
    return select("#", string.byte(""))
end

function test_string_pack_lua54(expected_int_size, expected_long_size)
    if #string.pack("i", 42) ~= expected_int_size then return 1 end
    if string.packsize("I") ~= expected_int_size then return 2 end
    if #string.pack("l", 42) ~= expected_long_size then return 3 end
    if string.packsize("L") ~= expected_long_size then return 4 end
    if #string.pack("x") ~= 1 then return 5 end
    if string.packsize("Xb") ~= 0 then return 6 end
    if string.unpack("", "hi") ~= 1 then return 7 end
    return 0
end

function test_string_byte_empty_bad_index()
    return string.byte("", true)
end

function test_string_pack_signed_overflow()
    return string.pack("b", 200)
end

function test_string_pack_unsigned_overflow()
    return string.pack("I1", 256)
end

function test_string_pack_z_with_zero()
    return string.pack("z", "a\0b")
end

function test_string_unpack_unfinished_z()
    return string.unpack("z", "abc")
end

function test_string_packsize_variable()
    return string.packsize("z")
end

add_executable(bit_stream_stress stress.cpp)
target_link_libraries(bit_stream_stress PRIVATE nalchi)
target_compile_options(bit_stream_stress PRIVATE ${nalchi_compile_options})
target_link_options(bit_stream_stress PRIVATE ${nalchi_link_options})

add_test(test_bit_stream_stress bit_stream_stress)
set_tests_properties(test_bit_stream_stress PROPERTIES TIMEOUT 0)

add_executable(execute_order)
target_sources(execute_order
    PRIVATE
        execute_order.cpp
)

target_link_libraries(execute_order
    PRIVATE
        cpp_crypto_algos_lib
)

set_target_properties(execute_order
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

clang_format_target(TARGET execute_order)
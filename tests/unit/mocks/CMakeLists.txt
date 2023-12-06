
add_library(pool_party_mocks INTERFACE)
target_include_directories(pool_party_mocks INTERFACE ${CMAKE_SOURCE_DIR}/tests/unit/)

target_compile_options(pool_party_mocks INTERFACE ${WARNING_FLAGS})
target_link_libraries(pool_party_mocks INTERFACE gtest gmock gtest_main)
set_target_properties(pool_party_mocks PROPERTIES
                      CXX_CLANG_TIDY "" # testcode excluded from clang-tidy run due to findings in gtest macros
                      FOLDER tests
)

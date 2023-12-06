add_subdirectory(mocks)

add_executable(poolparty_unit_tests
               thread_factory_tests.cpp
               thread_pool_tests.cpp
               sync_tests.cpp
)
target_compile_options(poolparty_unit_tests PRIVATE ${WARNING_FLAGS})
target_link_libraries(poolparty_unit_tests PRIVATE pool_party pool_party_mocks gtest gmock gtest_main)
gtest_discover_tests(poolparty_unit_tests)
set_target_properties(poolparty_unit_tests PROPERTIES
                      CXX_CLANG_TIDY "" # testcode excluded from clang-tidy run due to findings in gtest macros
                      FOLDER tests
)

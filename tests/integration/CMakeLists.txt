add_executable(poolparty_integration_tests
    thread_pool_tests.cpp
)
target_compile_options(poolparty_integration_tests PRIVATE ${WARNING_FLAGS})
target_link_libraries(poolparty_integration_tests PRIVATE pool_party gtest gmock gtest_main)
gtest_discover_tests(poolparty_integration_tests)
set_target_properties(poolparty_integration_tests PROPERTIES
                      CXX_CLANG_TIDY "" # testcode excluded from clang-tidy run due to findings in gtest macros
                      FOLDER tests
)



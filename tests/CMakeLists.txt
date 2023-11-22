add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

# hide gtest variables in ccmake configuration
mark_as_advanced(BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
                 gmock_build_tests gtest_build_samples gtest_build_tests
                 gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gmock gmock_main gtest gtest_main PROPERTIES
                      CXX_CLANG_TIDY "" # exclude gtest files from clang-tidy checks
                      FOLDER extern
)

add_executable(poolparty_tests
               thread_factory_tests.cpp
               thread_joiner_tests.cpp
)
target_compile_options(poolparty_tests PRIVATE ${WARNING_FLAGS})
target_link_libraries(poolparty_tests PRIVATE pool_party gtest gmock gtest_main)
gtest_discover_tests(poolparty_tests)
set_target_properties(poolparty_tests PROPERTIES
                      CXX_CLANG_TIDY "" # testcode excluded from clang-tidy run due to findings in gtest macros
                      FOLDER tests
)

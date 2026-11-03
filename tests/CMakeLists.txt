# Copyright © 2026 CCP ehf.

find_package(GTest CONFIG REQUIRED)

add_executable(test_exe test.cpp)
target_link_libraries(test_exe PRIVATE GTest::gtest GTest::gtest_main platform_pdm_proto_wrapper)
set_property(TARGET test_exe PROPERTY CXX_STANDARD 17)

include(GoogleTest)
gtest_discover_tests(test_exe DISCOVERY_MODE PRE_TEST)

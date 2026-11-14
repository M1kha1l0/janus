add_executable(janus_tests
    test_sanity.cpp
)

target_link_libraries(janus_tests
    PRIVATE
        janus_core
        janus_warnings
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(janus_tests)
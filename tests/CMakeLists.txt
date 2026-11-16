add_executable(janus_tests
    test_sanity.cpp
    ByteCursor-test.cpp
    TlsSniffer-test.cpp
)

target_include_directories(janus_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(janus_tests
    PRIVATE
        janus_core
        janus_warnings
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(janus_tests)
# Tests CMakeLists.txt for Cardano IoT SDK

find_package(GTest REQUIRED)

# Fix RPATH for Google Test on macOS
if(APPLE)
    set_target_properties(GTest::gtest PROPERTIES
        INTERFACE_LINK_OPTIONS "-Wl,-rpath,${GTest_DIR}/../../../lib"
    )
    set_target_properties(GTest::gtest_main PROPERTIES
        INTERFACE_LINK_OPTIONS "-Wl,-rpath,${GTest_DIR}/../../../lib"
    )
endif()

# Test utility library
add_library(test_utils
    utils/test_utils.cpp
)
target_link_libraries(test_utils cardano_iot_sdk GTest::gtest)

# Device Manager Tests
add_executable(device_manager_tests
    device_manager_tests.cpp
)
target_link_libraries(device_manager_tests 
    cardano_iot_sdk 
    test_utils
    GTest::gtest_main
)

# Security/Authentication Tests  
add_executable(security_tests
    security_tests.cpp
)
target_link_libraries(security_tests 
    cardano_iot_sdk 
    test_utils
    GTest::gtest_main
)

# Power Manager Tests
add_executable(power_manager_tests
    power_manager_tests.cpp
)
target_link_libraries(power_manager_tests 
    cardano_iot_sdk 
    test_utils
    GTest::gtest_main
)

# Network Tests
add_executable(network_tests
    network_tests.cpp
)
target_link_libraries(network_tests 
    cardano_iot_sdk 
    test_utils
    GTest::gtest_main
)

# Data Provenance Tests
add_executable(data_provenance_tests
    data_provenance_tests.cpp
)
target_link_libraries(data_provenance_tests 
    cardano_iot_sdk 
    test_utils
    GTest::gtest_main
)

# Integration Tests
add_executable(integration_tests
    integration_tests.cpp
)
target_link_libraries(integration_tests 
    cardano_iot_sdk 
    test_utils
    GTest::gtest_main
)

# Register tests with CTest
add_test(NAME DeviceManagerTests COMMAND device_manager_tests)
add_test(NAME SecurityTests COMMAND security_tests)
add_test(NAME PowerManagerTests COMMAND power_manager_tests)
add_test(NAME NetworkTests COMMAND network_tests)
add_test(NAME DataProvenanceTests COMMAND data_provenance_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)

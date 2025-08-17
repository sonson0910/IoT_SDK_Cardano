cmake_minimum_required(VERSION 3.16)

project(CardanoIoTSDK VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_DOCS "Build documentation" OFF)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Include directories
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

# Source files
set(SOURCES
    src/cardano_iot.cpp
    src/core/device_manager.cpp
    src/core/crypto_manager.cpp
    src/core/transaction_manager.cpp
    src/core/smart_contract_interface.cpp
    src/utils/logger.cpp
    src/utils/config.cpp
    src/energy/power_manager.cpp
    src/network/cardano_client.cpp
    src/network/p2p_network.cpp
    src/security/authentication.cpp
    src/security/encryption.cpp
    src/data/data_provenance.cpp
    src/identity/did.cpp
    src/security/attestation.cpp
)

# Headers
set(HEADERS
    include/cardano_iot/core/device_manager.h
    include/cardano_iot/core/crypto_manager.h
    include/cardano_iot/core/transaction_manager.h
    include/cardano_iot/core/smart_contract_interface.h
    include/cardano_iot/utils/logger.h
    include/cardano_iot/utils/config.h
    include/cardano_iot/energy/power_manager.h
    include/cardano_iot/network/cardano_client.h
    include/cardano_iot/network/p2p_network.h
    include/cardano_iot/security/authentication.h
    include/cardano_iot/security/encryption.h
    include/cardano_iot/data/data_provenance.h
    include/cardano_iot/cardano_iot.h
    include/cardano_iot/network/network_utils.h
    include/cardano_iot/identity/did.h
    include/cardano_iot/security/attestation.h
)

# Create main library
add_library(cardano_iot_sdk ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(cardano_iot_sdk 
    OpenSSL::SSL 
    OpenSSL::Crypto
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Set include directories for the target
target_include_directories(cardano_iot_sdk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(cardano_iot_sdk PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install configuration
install(TARGETS cardano_iot_sdk
    EXPORT CardanoIoTSDKTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT CardanoIoTSDKTargets
    FILE CardanoIoTSDKTargets.cmake
    NAMESPACE CardanoIoTSDK::
    DESTINATION lib/cmake/CardanoIoTSDK
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    CardanoIoTSDKConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

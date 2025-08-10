# Examples CMakeLists.txt for Cardano IoT SDK

# Basic device registration example
add_executable(device_registration
    device_registration.cpp
)
target_link_libraries(device_registration cardano_iot_sdk)

# IoT data submission example
add_executable(data_submission
    data_submission.cpp
)
target_link_libraries(data_submission cardano_iot_sdk)

# Smart contract integration example
add_executable(smart_contract_demo
    smart_contract_demo.cpp
)
target_link_libraries(smart_contract_demo cardano_iot_sdk)

# Energy monitoring example
add_executable(energy_monitoring
    energy_monitoring.cpp
)
target_link_libraries(energy_monitoring cardano_iot_sdk)

# P2P communication example
add_executable(p2p_communication
    p2p_communication.cpp
)
target_link_libraries(p2p_communication cardano_iot_sdk)

# Data provenance tracking example
add_executable(data_provenance_demo
    data_provenance_demo.cpp
)
target_link_libraries(data_provenance_demo cardano_iot_sdk)

# Copy example configs
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configs/example_config.json 
               ${CMAKE_CURRENT_BINARY_DIR}/example_config.json COPYONLY)

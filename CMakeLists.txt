cmake_minimum_required(VERSION 3.16)
project(drone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# RF24 submodule'ünü ekle (external/RF24 altında)
add_subdirectory(external/RF24)

# Tek executable: drone, kaynaklara mpu6050.cpp de ekleniyor
add_executable(drone
    src/drone.cpp
    src/mpu6050.cpp
)

# Başlık dizinleri: proje include/ ve RF24 include/
target_include_directories(drone PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/RF24
)

# RF24 kütüphanesini linkle
target_link_libraries(drone PRIVATE
    rf24
)

# compile_commands.json için symlink
add_custom_command(TARGET drone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
)

# "make run" ile hızlıca çalıştırmak için
add_custom_target(run
    COMMAND $<TARGET_FILE:drone>
    DEPENDS drone
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


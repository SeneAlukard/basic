cmake_minimum_required(VERSION 3.16)
project(drone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# rf24 submodule'ünü ekle (external/rf24 altında)
add_subdirectory(external/rf24)

# Tek executable: drone, kaynaklara mpu6050.cpp de ekleniyor
add_executable(drone
    src/drone.cpp
    src/mpu6050.cpp
)

add_executable(gbs
    src/gbs.cpp
)

# Başlık dizinleri: proje include/ ve rf24 include/
target_include_directories(drone PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/rf24
)

target_include_directories(gbs PRIVATE
  $(CMAKE_SOURCE_DIR)/include
  $(CMAKE_SOURCE_DIR)/external/rf24
)
target_link_libraries(gbs PRIVATE
  rf24
)

# rf24 kütüphanesini linkle
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


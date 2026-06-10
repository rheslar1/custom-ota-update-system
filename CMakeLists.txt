cmake_minimum_required(VERSION 3.16)
project(custom_ota_update_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(ota_core
  src/OtaUpdate.cpp
)
target_include_directories(ota_core PUBLIC include)
target_compile_options(ota_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ota_core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

add_executable(ota_update_tests tests/OtaUpdateTests.cpp)
target_link_libraries(ota_update_tests PRIVATE ota_core)
target_compile_options(ota_update_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME ota_update_tests COMMAND ota_update_tests)

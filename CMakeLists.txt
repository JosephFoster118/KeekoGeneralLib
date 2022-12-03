cmake_minimum_required(VERSION 3.13)
project(KeekoGeneralLib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

include_directories(include ${FREERTOS_CFG_DIRECTORY})
set(SOURCES
    src/KeekoMessage.cpp
    src/CRC.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)


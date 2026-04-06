cmake_minimum_required(VERSION 3.15...3.30)
project(BinanceLOBEngine VERSION 1.0)

# Enforce Modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress warnings by telling CMake to use the new Boost search policy
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Find necessary Arch Linux packages 
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

# Adding header files to the include path
include_directories(include)

# Defining source files
set(SOURCES
    src/main.cpp
    src/websocket.cpp 
    src/engine.cpp
)

# Create executable
add_executable(lob_engine ${SOURCES})

# Link OpenSSL, standard threading, and Boost headers
target_link_libraries(lob_engine 
    Boost::headers
    OpenSSL::SSL 
    OpenSSL::Crypto 
    pthread
)

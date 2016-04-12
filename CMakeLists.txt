cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(http-cpp VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##############################################################################
# CMake options
##############################################################################

###########################################################
# Setup output folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

##############################################################################
# Setup include paths.
##############################################################################

###########################################################
# Include boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_COMPILER "-clang37")
find_package(Boost COMPONENTS system filesystem REQUIRED)
if(${Boost_FOUND})
    add_library(boost INTERFACE IMPORTED)
    set_property(TARGET boost PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

    add_library(boost-system STATIC IMPORTED)
    set_property(TARGET boost-system PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    set_property(TARGET boost-system PROPERTY IMPORTED_LOCATION "${Boost_SYSTEM_LIBRARY}")
    set_property(TARGET boost-system PROPERTY IMPORTED_LOCATION_DEBUG "${Boost_SYSTEM_LIBRARY_DEBUG}")
    set_property(TARGET boost-system PROPERTY IMPORTED_LOCATION_RELEASE "${Boost_SYSTEM_LIBRARY_RELEASE}")

    add_library(boost-filesystem STATIC IMPORTED)
    set_property(TARGET boost-filesystem PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    set_property(TARGET boost-filesystem PROPERTY IMPORTED_LOCATION "${Boost_FILESYSTEM_LIBRARY}")
    set_property(TARGET boost-filesystem PROPERTY IMPORTED_LOCATION_DEBUG "${Boost_FILESYSTEM_LIBRARY_DEBUG}")
    set_property(TARGET boost-filesystem PROPERTY IMPORTED_LOCATION_RELEASE "${Boost_FILESYSTEM_LIBRARY_RELEASE}")
endif()

###########################################################
# Include zeroMQ
find_package(ZMQ REQUIRED)
if(${ZMQ_FOUND})
    add_library(zmq STATIC IMPORTED)
    set_property(TARGET zmq PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${ZMQ_INCLUDE_DIRS})
    set_property(TARGET zmq PROPERTY IMPORTED_LOCATION ${ZMQ_LIBRARIES})
endif()

###########################################################
# Include cppzmq
add_library(zmqcpp INTERFACE IMPORTED)
set_property(TARGET zmqcpp PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/cppzmq")

##############################################################################
# Set compiler options
##############################################################################

if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(STANDARD_LIBRARY "stdc++")
endif()

if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(THREADING_LIBRARY "pthread")
endif()

##############################################################################
# Prepare subdirectories
##############################################################################

add_subdirectory(libhttp-cpp)
add_subdirectory(librest)
add_subdirectory(example)

##############################################################################
# Create executable
##############################################################################

add_executable(http-cpp
    src/main.cpp
    src/http_server.h
    src/http_server.cpp
    src/http_worker.h
    src/http_worker.cpp
    src/http_website.hpp
    src/http_website.h
    src/http_website.cpp
    src/zmq_utility.hpp
    src/logger.hpp
    src/logger.cpp
    src/runnable.h
    src/identity.h
)

# Compiler requirement for the library.
set_property(TARGET http-cpp PROPERTY CXX_STANDARD 14)

target_link_libraries(http-cpp
    boost
    boost-system
    boost-filesystem
    zmq
    zmqcpp
    libhttp-cpp
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

if (WIN32)
    set_property(TARGET http-cpp APPEND PROPERTY COMPILE_DEFINITIONS BOOST_USE_WINDOWS_H)
endif()

##############################################################################
# Post build commands
##############################################################################

add_custom_command(TARGET http-cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZMQ_LIBRARIES} $<TARGET_FILE_DIR:http-cpp>
)

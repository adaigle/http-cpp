cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
#project(http_example_custom VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##############################################################################
# Setup include paths.
##############################################################################

###########################################################
# Include boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_COMPILER "-clang37")
find_package(Boost 1.60 REQUIRED)
if(${Boost_FOUND})
    if (NOT TARGET boost)
        add_library(boost INTERFACE IMPORTED)
        set_property(TARGET boost PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    endif()
endif()

###########################################################
# Include boost.dll
add_library(boost-dll INTERFACE IMPORTED)
set_property(TARGET boost-dll PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/ext/dll/include")

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
# Create executable
##############################################################################

add_library(custom_webservice SHARED EXCLUDE_FROM_ALL
    "${PROJECT_SOURCE_DIR}/libhttp-cpp/include/interface/http_resource.h"
    include/custom_resource.h
    src/custom_resource.cpp
)
add_dependencies(examples custom_webservice)

get_property(EXAMPLE_ARCHIVE_OUTPUT_DIRECTORY TARGET examples PROPERTY ARCHIVE_OUTPUT_DIRECTORY)
set_property(TARGET custom_webservice PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${EXAMPLE_ARCHIVE_OUTPUT_DIRECTORY}")
get_property(EXAMPLE_RUNTIME_OUTPUT_DIRECTORY TARGET examples PROPERTY RUNTIME_OUTPUT_DIRECTORY)
set_property(TARGET custom_webservice PROPERTY RUNTIME_OUTPUT_DIRECTORY "${EXAMPLE_RUNTIME_OUTPUT_DIRECTORY}")
get_property(EXAMPLE_LIBRARY_OUTPUT_DIRECTORY TARGET examples PROPERTY LIBRARY_OUTPUT_DIRECTORY)
set_property(TARGET custom_webservice PROPERTY LIBRARY_OUTPUT_DIRECTORY "${EXAMPLE_LIBRARY_OUTPUT_DIRECTORY}")

set_property(TARGET custom_webservice PROPERTY PREFIX "")
set_property(TARGET custom_webservice APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_property(TARGET custom_webservice APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_property(TARGET custom_webservice APPEND PROPERTY INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/libhttp-cpp/include")

# Compiler requirement for the library.
set_target_properties(custom_webservice PROPERTIES CXX_STANDARD 14)

target_link_libraries(custom_webservice
    boost
    boost-dll
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

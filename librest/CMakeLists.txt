cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(librest VERSION 0.1 LANGUAGES CXX)

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
set_property(TARGET boost-dll PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ext/dll/include")

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

add_library(librest SHARED
    ../libhttp-cpp/src/http_resource.h
    ../libhttp-cpp/src/http_resource.cpp
    include/rest_resource.h
    src/rest_resource.cpp
)
set_property(TARGET librest PROPERTY PREFIX "")
set_property(TARGET librest APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_property(TARGET librest APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Compiler requirement for the library.
set_target_properties(librest PROPERTIES CXX_STANDARD 14)

target_link_libraries(librest
    boost
    boost-dll
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

##############################################################################
# Prepare subdirectories
##############################################################################


##############################################################################
# Post build commands
##############################################################################

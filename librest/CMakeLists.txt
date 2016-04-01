cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

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

add_library(librest STATIC
    include/http_router.h
    include/rest_service.h
    include/rest_resource.h
    src/http_router.cpp
)
set_property(TARGET librest PROPERTY PREFIX "")
set_property(TARGET librest APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_property(TARGET librest APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_property(TARGET librest APPEND PROPERTY INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/libhttp-cpp/include")
set_property(TARGET librest APPEND PROPERTY IMPORTED_LOCATION "$<TARGET_LINKER_FILE_NAME:librest>")

# Compiler requirement for the library.
set_target_properties(librest PROPERTIES CXX_STANDARD 14)

target_link_libraries(librest
    boost
    boost-dll
    libhttp-interface
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

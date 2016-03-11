cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(http VERSION 0.1 LANGUAGES CXX)

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
# Include zeroMQ
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
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

###########################################################
# Include libmagic
set(libmagic_ROOT_DIR $ENV{libmagic_ROOT})
find_package(libmagic)
if (${libmagic_FOUND})
    add_library(libmagic SHARED IMPORTED)
    set_property(TARGET libmagic PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${libmagic_INCLUDES})
    set_property(TARGET libmagic PROPERTY IMPORTED_LOCATION ${libmagic_LIBRARIES})
    set_property(TARGET libmagic PROPERTY IMPORTED_IMPLIB ${libmagic_LIBRARIES})
    set_property(TARGET libmagic PROPERTY IMPORTED_LINK_DEPENDENT_LIBRARIES ${libmagic_DEPENDENCIES})

    set(libmagic_MAGIC_FILE "magic/magic.mgc")
endif()

if(NOT ${libmagic_FOUND})
    message(WARNING "libmagic could NOT be found, content-type detection will be disabled.")
endif()

###########################################################
# Include boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_COMPILER "-clang37")
find_package(Boost 1.60 COMPONENTS system filesystem REQUIRED)
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

add_executable(http
    src/main.cpp
    src/http_server.h
    src/http_server.cpp
    src/virtual_website.hpp
    src/virtual_website.h
    src/virtual_website.cpp
    src/http_structure.hpp
    src/http_structure.cpp
    src/zmq_utility.hpp
    src/logger.hpp
    src/logger.cpp
)

if(${libmagic_FOUND})
    set_property(TARGET http APPEND PROPERTY COMPILE_DEFINITIONS HAVE_LIBMAGIC)
    set_property(TARGET http APPEND PROPERTY COMPILE_DEFINITIONS "LIBMAGIC_MAGIC_FILE=\"${libmagic_MAGIC_FILE}\"")
endif()

# Compiler requirement for the library.
set_target_properties(http PROPERTIES CXX_STANDARD 14)

target_link_libraries(http
    boost
    boost-system
    boost-filesystem
    zmq
    zmqcpp
    libmagic
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

##############################################################################
# Post build commands
##############################################################################

add_custom_command(TARGET http POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZMQ_LIBRARIES} $<TARGET_FILE_DIR:http>
)

if(${libmagic_FOUND})
    add_custom_command(TARGET http POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${libmagic_LIBRARIES} $<TARGET_FILE_DIR:http>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${libmagic_MAGIC_DB}" "$<TARGET_FILE_DIR:http>/${libmagic_MAGIC_FILE}"
    )

    foreach(dependency ${libmagic_DEPENDENCIES})
        add_custom_command(TARGET http POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dependency} $<TARGET_FILE_DIR:http>
      )
    endforeach(dependency)
endif()

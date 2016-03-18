cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(libhttp-cpp VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##############################################################################
# Setup include paths.
##############################################################################

###########################################################
# Include boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_COMPILER "-clang37")
find_package(Boost 1.60 COMPONENTS system filesystem date_time REQUIRED)
if(${Boost_FOUND})
    if (NOT TARGET boost)
        add_library(boost INTERFACE IMPORTED)
        set_property(TARGET boost PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    endif()

    if (NOT TARGET boost-system)
        add_library(boost-system STATIC IMPORTED)
        set_property(TARGET boost-system PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
        set_property(TARGET boost-system PROPERTY IMPORTED_LOCATION "${Boost_SYSTEM_LIBRARY}")
        set_property(TARGET boost-system PROPERTY IMPORTED_LOCATION_DEBUG "${Boost_SYSTEM_LIBRARY_DEBUG}")
        set_property(TARGET boost-system PROPERTY IMPORTED_LOCATION_RELEASE "${Boost_SYSTEM_LIBRARY_RELEASE}")
    endif()

    if (NOT TARGET boost-filesystem)
        add_library(boost-filesystem STATIC IMPORTED)
        set_property(TARGET boost-filesystem PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
        set_property(TARGET boost-filesystem PROPERTY IMPORTED_LOCATION "${Boost_FILESYSTEM_LIBRARY}")
        set_property(TARGET boost-filesystem PROPERTY IMPORTED_LOCATION_DEBUG "${Boost_FILESYSTEM_LIBRARY_DEBUG}")
        set_property(TARGET boost-filesystem PROPERTY IMPORTED_LOCATION_RELEASE "${Boost_FILESYSTEM_LIBRARY_RELEASE}")
    endif()

    if (NOT TARGET boost-date_time)
        add_library(boost-date_time STATIC IMPORTED)
        set_property(TARGET boost-date_time PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
        set_property(TARGET boost-date_time PROPERTY IMPORTED_LOCATION "${Boost_DATE_TIME_LIBRARY}")
        set_property(TARGET boost-date_time PROPERTY IMPORTED_LOCATION_DEBUG "${Boost_DATE_TIME_LIBRARY_DEBUG}")
        set_property(TARGET boost-date_time PROPERTY IMPORTED_LOCATION_RELEASE "${Boost_DATE_TIME_LIBRARY_RELEASE}")
    endif()
endif()

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
# Setup tests
##############################################################################
enable_testing()
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Build and then run all the tests and examples."
)

##############################################################################
# Create executable
##############################################################################

add_library(libhttp-cpp SHARED
    include/http_service.h
    include/http_structure.hpp
    src/http_service.cpp
    src/http_structure.cpp
    src/http_resource.hpp
    src/http_resource.cpp
    src/logger.hpp
    src/logger.cpp
    src/main.cpp
)
set_property(TARGET libhttp-cpp PROPERTY PREFIX "")
set_property(TARGET libhttp-cpp PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(${libmagic_FOUND})
    set_property(TARGET libhttp-cpp APPEND PROPERTY COMPILE_DEFINITIONS HAVE_LIBMAGIC)
    set_property(TARGET libhttp-cpp APPEND PROPERTY COMPILE_DEFINITIONS "LIBMAGIC_MAGIC_FILE=\"${libmagic_MAGIC_FILE}\"")
endif()

# Compiler requirement for the library.
set_target_properties(libhttp-cpp PROPERTIES CXX_STANDARD 14)

target_link_libraries(libhttp-cpp
    boost
    boost-system
    boost-filesystem
    boost-date_time
    libmagic
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

if (WIN32)
    set_property(TARGET libhttp-cpp APPEND PROPERTY COMPILE_DEFINITIONS BOOST_USE_WINDOWS_H)
endif()

##############################################################################
# Prepare subdirectories
##############################################################################

add_subdirectory(test)

##############################################################################
# Post build commands
##############################################################################

if(${libmagic_FOUND})
    add_custom_command(TARGET libhttp-cpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${libmagic_LIBRARIES} $<TARGET_FILE_DIR:libhttp-cpp>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${libmagic_MAGIC_DB}" "$<TARGET_FILE_DIR:libhttp-cpp>/${libmagic_MAGIC_FILE}"
    )

    foreach(dependency ${libmagic_DEPENDENCIES})
        add_custom_command(TARGET libhttp-cpp POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dependency} $<TARGET_FILE_DIR:libhttp-cpp>
      )
    endforeach(dependency)
endif()

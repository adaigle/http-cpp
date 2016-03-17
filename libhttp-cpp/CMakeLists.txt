cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(libhttp-cpp VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##############################################################################
# Setup include paths.
##############################################################################

############################################################
## Include libmagic
#set(libmagic_ROOT_DIR $ENV{libmagic_ROOT})
#find_package(libmagic)
#if (${libmagic_FOUND})
#    add_library(libmagic SHARED IMPORTED)
#    set_property(TARGET libmagic PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${libmagic_INCLUDES})
#    set_property(TARGET libmagic PROPERTY IMPORTED_LOCATION ${libmagic_LIBRARIES})
#    set_property(TARGET libmagic PROPERTY IMPORTED_IMPLIB ${libmagic_LIBRARIES})
#    set_property(TARGET libmagic PROPERTY IMPORTED_LINK_DEPENDENT_LIBRARIES ${libmagic_DEPENDENCIES})
#
#    set(libmagic_MAGIC_FILE "magic/magic.mgc")
#endif()
#
#if(NOT ${libmagic_FOUND})
#    message(WARNING "libmagic could NOT be found, content-type detection will be disabled.")
#endif()

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

add_library(libhttp-cpp
    src/main.cpp
)
set_target_properties(libhttp-cpp PROPERTIES PREFIX "")

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

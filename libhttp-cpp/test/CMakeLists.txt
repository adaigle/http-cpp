cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(http_test VERSION 0.1 LANGUAGES CXX)

add_custom_target(tests COMMENT "Build all the unit tests.")
add_dependencies(check tests)

##############################################################################
# Setup include paths.
##############################################################################

###########################################################
# Include googletest
find_package(GTest REQUIRED)
if(GTest_FOUND)
    add_library(gtest STATIC IMPORTED)
    set_property(TARGET gtest PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIRS})
    set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARY})

    add_library(gtest_main STATIC IMPORTED)
    set_property(TARGET gtest_main PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIRS})
    set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY})

    set(GTEST_LIBRARIES gtest gtest_main)
else()
    message(WARNING "GTest NOT found.")
endif()

##############################################################################
# Create executable
##############################################################################

add_executable(http_conformance_test EXCLUDE_FROM_ALL
    http/method.cpp
)
add_test(http_conformance_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/http_conformance_test)
add_dependencies(tests http_conformance_test)

# Compiler requirement for the library.
set_target_properties(http_conformance_test PROPERTIES CXX_STANDARD 14)

target_link_libraries(http_conformance_test
    zmq
    zmqcpp
    gtest
    gtest_main
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

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
if(${GTEST_FOUND})
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
# Set compiler options
##############################################################################

if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(STANDARD_LIBRARY "stdc++")
endif()

if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(THREADING_LIBRARY "pthread")
endif()

set(TEST_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test")

##############################################################################
# Create executable
##############################################################################

add_executable(http_conformance_test EXCLUDE_FROM_ALL
    http/method.cpp
)
set_target_properties(http_conformance_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_WORKING_DIRECTORY})
add_test(NAME http_conformance_test
    COMMAND http_conformance_test
    WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
)
add_dependencies(tests http_conformance_test)

# Compiler requirement for the library.
set_property(TARGET http_conformance_test PROPERTY CXX_STANDARD 14)

target_link_libraries(http_conformance_test
    gtest
    gtest_main
    libhttp-cpp
    ${THREADING_LIBRARY}
    ${STANDARD_LIBRARY}
)

add_custom_command(TARGET http_conformance_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:libhttp-cpp>/$<TARGET_FILE_NAME:libhttp-cpp> ${TEST_WORKING_DIRECTORY}
)
if(${libmagic_FOUND})
    add_custom_command(TARGET http_conformance_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${libmagic_LIBRARIES} ${TEST_WORKING_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:libhttp-cpp>/${libmagic_MAGIC_FILE}" ${TEST_WORKING_DIRECTORY}/${libmagic_MAGIC_FILE}
    )

    foreach(dependency ${libmagic_DEPENDENCIES})
        add_custom_command(TARGET http_conformance_test POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dependency} ${TEST_WORKING_DIRECTORY}
      )
    endforeach(dependency)
endif()


file(GLOB test_file_folders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/files ${CMAKE_CURRENT_SOURCE_DIR}/files/*)
foreach(test_folder ${test_file_folders})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/files/${test_folder}")
        add_custom_command(TARGET http_conformance_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/files/${test_folder}
            ${TEST_WORKING_DIRECTORY}/${test_folder}
        )
    endif()
endforeach()

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
#project(http_example VERSION 0.1 LANGUAGES CXX)

add_custom_target(examples COMMENT "Build all examples.")
set_property(TARGET examples PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/example")
set_property(TARGET examples PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/example")
set_property(TARGET examples PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/example")

##############################################################################
# Prepare subdirectories
##############################################################################

add_subdirectory(custom_webservice)
add_subdirectory(rest_webservice)

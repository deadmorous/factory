cmake_minimum_required (VERSION 3.1.3) 

project(factory_pkg)

set (CMAKE_CXX_STANDARD 14)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set (BUILD_SHARED_LIBS true)

include_directories(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)


install(DIRECTORY include/factory DESTINATION include)
install(FILES cmake/FindFactory.cmake DESTINATION cmake)


option (BUILD_FACTORY_EXAMPLE "Build an example project for factory" OFF)
if (BUILD_FACTORY_EXAMPLE)
    add_subdirectory(factory_example)
endif()

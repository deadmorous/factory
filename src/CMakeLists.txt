cmake_minimum_required (VERSION 3.1.3) 

project(factory)

file(GLOB HEADER_FILES
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.hpp
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h
)

aux_source_directory(. SOURCE_FILES)
add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
  COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG>
)


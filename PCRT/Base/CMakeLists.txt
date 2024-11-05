cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME PCRTBase)
project(${PROJ_NAME})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (PCRT_DIR "${CMAKE_SOURCE_DIR}/PCRT/")

set(SRC_FILES
	${SRC_DIR}/Platforms/Windows/Entry.cpp
	${SRC_DIR}/Platforms/Windows/Required.cpp
	)

add_library(${PROJ_NAME} ${SRC_FILES})

target_link_libraries(${PROJ_NAME} PRIVATE user32 Kernel32)
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PCRT_DIR})

target_compile_definitions(${PROJ_NAME} PRIVATE PCRT_BASE_PROJECT)
target_compile_options(${PROJ_NAME} PUBLIC /FI ${SRC_DIR}/Platforms/Windows/Required.h)

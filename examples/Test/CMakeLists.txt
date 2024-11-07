cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME Game)
project(${PROJ_NAME})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Test.cpp
	)

add_library(${PROJ_NAME} SHARED ${SRC_FILES})

target_link_libraries(${PROJ_NAME} PRIVATE PEngine)
target_compile_definitions(${PROJ_NAME} PRIVATE GAME_PROJECT LOG_LEVEL_INFO LOG_LEVEL_WARN LOG_LEVEL_ERROR)

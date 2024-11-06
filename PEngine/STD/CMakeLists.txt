cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME PSTD)
project(${PROJ_NAME})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set (PENGINE_DIR "${CMAKE_SOURCE_DIR}/PEngine")

set(SRC_FILES
	${SRC_DIR}/Platforms/Windows/Entry.cpp
	${SRC_DIR}/Platforms/Windows/Required.cpp
	${SRC_DIR}/Platforms/Windows/Memory.cpp
	${SRC_DIR}/Platforms/Windows/Console.cpp
	${SRC_DIR}/Platforms/Windows/FileIO.cpp
	${SRC_DIR}/String.cpp
	${SRC_DIR}/Arena.cpp
	${SRC_DIR}/Memory.cpp
	${SRC_DIR}/Vector.cpp
	${SRC_DIR}/Matrix.cpp
	${SRC_DIR}/Math.cpp
	)

add_library(${PROJ_NAME} ${SRC_FILES})

target_link_libraries(${PROJ_NAME} PRIVATE user32 Kernel32 )
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${SRC_DIR} ${PENGINE_DIR})
target_include_directories(${PROJ_NAME} PUBLIC ${INCLUDE_DIR})

target_compile_definitions(${PROJ_NAME} PRIVATE PSTD_PROJECT)
target_compile_options(${PROJ_NAME} PUBLIC /FI ${SRC_DIR}/Platforms/Windows/Required.h)

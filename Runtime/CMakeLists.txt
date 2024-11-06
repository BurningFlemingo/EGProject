cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME PEngineRuntime)
project(${PROJ_NAME})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Main.cpp
	)

add_executable(${PROJ_NAME} ${SRC_FILES})

target_link_libraries(${PROJ_NAME} PRIVATE PenguiEngine)
target_compile_definitions(${PROJ_NAME} PRIVATE RUNTIME_PROJECT)

cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME PEngineRuntime)
project(${PROJ_NAME})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (ENGINE_SRC_DIR "${CMAKE_SOURCE_DIR}/PEngine/src")
set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Main.cpp
	)

add_executable(${PROJ_NAME} ${SRC_FILES})

target_link_libraries(${PROJ_NAME} PRIVATE PEngine)
target_compile_definitions(${PROJ_NAME} PRIVATE PENGINE_RUNTIME_PROJECT LOG_LEVEL_INFO LOG_LEVEL_WARN LOG_LEVEL_ERROR)
target_include_directories(${PROJ_NAME}
	PRIVATE ${ENGINE_SRC_DIR}
)

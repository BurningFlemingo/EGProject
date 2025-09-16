cmake_minimum_required(VERSION 3.8)

project(PEngineRuntime)

set(CMAKE_CXX_STANDARD 23)

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (ENGINE_SRC_DIR "${CMAKE_SOURCE_DIR}/PEngine/Engine/src")

set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Main.cpp
	)

add_executable(PEngineRuntime ${SRC_FILES})

target_link_libraries(PEngineRuntime PRIVATE PEngine)

target_include_directories(PEngineRuntime
	PRIVATE ${SRC_DIR} ${ENGINE_SRC_DIR}
)

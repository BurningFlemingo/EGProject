cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME PCRT)
project(${PROJ_NAME})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")

set(SRC_FILES
	${SRC_DIR}/Platforms/Windows/Memory.cpp
	${SRC_DIR}/Platforms/Windows/Console.cpp
	${SRC_DIR}/Platforms/Windows/FileIO.cpp
	${SRC_DIR}/String.cpp
	${SRC_DIR}/Arena.cpp
	${SRC_DIR}/Memory.cpp
	${SRC_DIR}/Vector.cpp
	${SRC_DIR}/Matrix.cpp
	${SRC_DIR}/Math.cpp
	${SRC_DIR}/Startup.cpp
	)

add_subdirectory(Base)

add_library(${PROJ_NAME} SHARED ${SRC_FILES})

target_link_libraries(${PROJ_NAME} PRIVATE PCRTBase user32 Kernel32 )
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJ_NAME} PUBLIC ${PUBLIC_DIR})

target_compile_definitions(${PROJ_NAME} PRIVATE PCRT_PROJECT)

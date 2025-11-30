cmake_minimum_required(VERSION 3.8)

project(PEngineRuntime)

set(CMAKE_CXX_STANDARD 23)

find_program(GLSLC NAMES glslc HINTS Vulkan::glslc)

set(SHADERS_SRC_DIR "${CMAKE_SOURCE_DIR}/assets/shaders")
set(SHADERS_BIN_DIR "${CMAKE_BINARY_DIR}/generated/shaders")

file(GLOB SHADERS
	"${SHADERS_SRC_DIR}/*.vert"
	"${SHADERS_SRC_DIR}/*.frag"
	"${SHADERS_SRC_DIR}/*.comp"
	"${SHADERS_SRC_DIR}/*.geom"
	"${SHADERS_SRC_DIR}/*.tesc"
	"${SHADERS_SRC_DIR}/*.tese")

file(MAKE_DIRECTORY "${SHADERS_BIN_DIR}")

foreach(SHADER ${SHADERS})
	get_filename_component(SHADER_NAME ${SHADER} NAME)
	set(SHADER_BIN_NAME "${SHADERS_BIN_DIR}/${SHADER_NAME}.spv")
	add_custom_command(
		DEPENDS "${SHADER}"
		OUTPUT "${SHADER_BIN_NAME}"
		COMMAND "${GLSLC}" "${SHADER}" "-o" "${SHADER_BIN_NAME}"
		COMMENT "Compiling ${SHADER_NAME}"
		VERBATIM)
	list(APPEND SPV_SHADERS "${SHADER_BIN_NAME}")
endforeach()

add_custom_target(build_shaders DEPENDS ${SPV_SHADERS})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (ENGINE_SRC_DIR "${CMAKE_SOURCE_DIR}/PEngine/Engine/src")

set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Main.cpp
	${SRC_DIR}/AssetCooker.cpp
	)

add_executable(PEngineRuntime ${SRC_FILES})

target_link_libraries(PEngineRuntime PRIVATE PEngine)
target_compile_definitions(PEngineRuntime PRIVATE LOG_LEVEL_INFO LOG_LEVEL_WARN LOG_LEVEL_ERROR)

target_include_directories(PEngineRuntime
	PRIVATE ${SRC_DIR} ${ENGINE_SRC_DIR}
)

add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	${CMAKE_SOURCE_DIR}/assets
	${CMAKE_BINARY_DIR}/assets
	COMMENT "Updating binary assets folder"
)

add_dependencies(copy_assets build_shaders)
add_dependencies(PEngineRuntime copy_assets)

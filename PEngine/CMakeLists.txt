cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME PEngine)
project(${PROJ_NAME})

find_package(VulkanHeaders CONFIG)
find_package(Vulkan REQUIRED)

find_program(GLSLC NAMES glslc HINTS Vulkan::glslc)

set(CMAKE_CXX_STANDARD 23)

set(SHADERS_SRC_DIR "${CMAKE_SOURCE_DIR}/assets/shaders")
set(SHADERS_BIN_DIR "${CMAKE_BINARY_DIR}/shaders")

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
		MAIN_DEPENDENCY "${SHADER_BIN_DIR}"
		DEPENDS "${SHADER}"
		OUTPUT "${SHADER_BIN_NAME}"
		COMMAND "${GLSLC}" "${SHADER}" "-o" "${SHADER_BIN_NAME}"
		COMMENT "Compiling ${SHADER_NAME}"
		VERBATIM)
	list(APPEND SPV_SHADERS "${SHADER_BIN_NAME}")
endforeach()

add_custom_target(build_shaders DEPENDS ${SPV_SHADERS})

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Engine/Engine.cpp
	${SRC_DIR}/Engine/Logging.cpp
	${SRC_DIR}/Engine/Platforms/Windows/Window.cpp
	${SRC_DIR}/Engine/Platforms/Windows/VulkanSurface.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Vulkan.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Debug/ValidationLayers.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Debug/Extensions.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Debug/DebugMessenger.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Extensions.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Instance.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Device.cpp
	${SRC_DIR}/Engine/Renderer/Vulkan/Swapchain.cpp

	${SRC_DIR}/Core/Platforms/Windows/Entry.cpp
	${SRC_DIR}/Core/Platforms/Windows/Required.cpp
	${SRC_DIR}/Core/Platforms/Windows/Memory.cpp
	${SRC_DIR}/Core/Platforms/Windows/Console.cpp
	${SRC_DIR}/Core/Platforms/Windows/FileIO.cpp
	${SRC_DIR}/Core/String.cpp
	${SRC_DIR}/Core/Arena.cpp
	${SRC_DIR}/Core/Memory.cpp
	${SRC_DIR}/Core/Vector.cpp
	${SRC_DIR}/Core/Matrix.cpp
	${SRC_DIR}/Core/Math.cpp
	${SRC_DIR}/Core/FileIO.cpp

	)

add_library(${PROJ_NAME} ${SRC_FILES})
target_link_libraries(${PROJ_NAME} PRIVATE Vulkan::Headers Vulkan::Vulkan user32 Kernel32)
target_include_directories(${PROJ_NAME}
	PRIVATE ${SRC_DIR}
	PUBLIC ${INCLUDE_DIR}
)
target_compile_definitions(${PROJ_NAME} PRIVATE PENGINE_PROJECT LOG_LEVEL_INFO LOG_LEVEL_WARN LOG_LEVEL_ERROR)
target_compile_options(${PROJ_NAME} PUBLIC /FI ${SRC_DIR}/Core/Platforms/Windows/Required.h)

add_custom_command(
	TARGET ${PROJ_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/compile_commands.json"
		"${CMAKE_BINARY_DIR}"
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to src directory"
    VERBATIM
)

add_dependencies(${PROJ_NAME} build_shaders)

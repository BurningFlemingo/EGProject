cmake_minimum_required(VERSION 3.8)

project(PEngine)

find_package(VulkanHeaders CONFIG)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 23)

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(BUILD_TYPE Release)
else()
	set(BUILD_TYPE Debug)
endif()

set(SRC_FILES
	${SRC_DIR}/Engine.cpp
	${SRC_DIR}/ECS.cpp
	${SRC_DIR}/Logging.cpp
	${SRC_DIR}/AssetLoader.cpp
	${SRC_DIR}/AssetManager.cpp
	${SRC_DIR}/Platforms/Windows/Window.cpp
	${SRC_DIR}/Platforms/Windows/VulkanSurface.cpp
	${SRC_DIR}/Renderer/Vulkan/Vulkan.cpp
	${SRC_DIR}/Renderer/Vulkan/Extensions.cpp
	${SRC_DIR}/Renderer/Vulkan/Instance.cpp
	${SRC_DIR}/Renderer/Vulkan/Device.cpp
	${SRC_DIR}/Renderer/Vulkan/Swapchain.cpp
	${SRC_DIR}/Renderer/Vulkan/Allocation.cpp
	${SRC_DIR}/Renderer/Vulkan/Pipeline.cpp
	${SRC_DIR}/Renderer/Vulkan/Commands.cpp
	
	${SRC_DIR}/Renderer/Vulkan/${BUILD_TYPE}/ValidationLayers.cpp
	${SRC_DIR}/Renderer/Vulkan/${BUILD_TYPE}/Extensions.cpp
	${SRC_DIR}/Renderer/Vulkan/${BUILD_TYPE}/DebugMessenger.cpp

	${SRC_DIR}/STD/Platforms/Windows/Entry.cpp
	${SRC_DIR}/STD/Platforms/Windows/Time.cpp
	${SRC_DIR}/STD/Platforms/Windows/Required.cpp
	${SRC_DIR}/STD/Platforms/Windows/Memory.cpp
	${SRC_DIR}/STD/Platforms/Windows/Console.cpp
	${SRC_DIR}/STD/Platforms/Windows/FileIO.cpp
	${SRC_DIR}/STD/Platforms/Windows/Assert.cpp
	${SRC_DIR}/STD/HashMap.cpp
	${SRC_DIR}/STD/Algorithm.cpp
	${SRC_DIR}/STD/String.cpp
	${SRC_DIR}/STD/Arena.cpp
	${SRC_DIR}/STD/Memory.cpp
	${SRC_DIR}/STD/Vector.cpp
	${SRC_DIR}/STD/Matrix.cpp
	${SRC_DIR}/STD/Math.cpp
	${SRC_DIR}/STD/FileIO.cpp
)


add_library(PEngine ${SRC_FILES})

target_link_libraries(PEngine PRIVATE Vulkan::Headers Vulkan::Vulkan user32 Kernel32)

target_include_directories(PEngine
	PRIVATE ${SRC_DIR}
	PUBLIC ${INCLUDE_DIR}
)

target_compile_definitions(PEngine PRIVATE PENGINE_PROJECT LOG_LEVEL_INFO LOG_LEVEL_WARN LOG_LEVEL_ERROR)
target_compile_options(PEngine PUBLIC /FI ${SRC_DIR}/STD/Platforms/Windows/Required.h)

add_custom_command(
	TARGET PEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/compile_commands.json"
		"${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to src directory"
    VERBATIM
)


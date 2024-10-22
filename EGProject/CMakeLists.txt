cmake_minimum_required(VERSION 3.8)

set (PROJ_NAME eg_project)
project(${PROJ_NAME})

find_package(VulkanHeaders CONFIG)
find_package(Vulkan REQUIRED)

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

set(SRC_FILES
	${SRC_DIR}/Main.cpp
	)

add_executable(${PROJ_NAME} ${SRC_FILES})
target_link_libraries(${PROJ_NAME} PRIVATE Vulkan::Headers Vulkan::Vulkan user32 Kernel32 CRT)

add_custom_command(
	TARGET ${PROJ_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}"
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to src directory"
    VERBATIM
)

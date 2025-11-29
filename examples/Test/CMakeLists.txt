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

if(MSVC)
	add_custom_target(clean_pdbs
        COMMAND ${CMAKE_COMMAND}
		-DBINARY_DIR=${CMAKE_BINARY_DIR}
		-P ${CMAKE_SOURCE_DIR}/scripts/remove_pdbs.cmake
        COMMENT "Removing out of date PDB files..."
        VERBATIM
    )
    add_dependencies(${PROJECT_NAME} clean_pdbs)
	
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
		-DPDB_FILE=$<TARGET_PDB_FILE:${PROJECT_NAME}>
		-DBINARY_DIR=$<TARGET_FILE_DIR:${PROJECT_NAME}>
		-DPROJ_NAME=${PROJECT_NAME}
		-P ${CMAKE_SOURCE_DIR}/scripts/timestamp_pdb.cmake
        COMMENT "Creating timestamped PDB"
        VERBATIM
    )
endif()

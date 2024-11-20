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

string(TIMESTAMP PDB_TIMESTAMP "%Y%m%d_%H%M%S")
set_target_properties(${PROJ_NAME} PROPERTIES
	LINK_FLAGS "/PDB:Game_${PDB_TIMESTAMP}.pdb" DEPENDS cleanPdbs
)

file(GLOB old_pdbs "${CMAKE_BINARY_DIR}/Game_*.pdb")
foreach(pdb ${old_pdbs})
    file(REMOVE "${pdb}")
endforeach()

add_custom_command(TARGET ${PROJ_NAME} PRE_BUILD
   COMMAND ${CMAKE_COMMAND} -E echo "Cleaning old PDBs..."
   COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/scripts/remove_pdbs.cmake || true
)

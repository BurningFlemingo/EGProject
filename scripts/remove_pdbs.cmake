file(GLOB oldPDBs "${CMAKE_BINARY_DIR}/Game_*.pdb")
foreach(pdb ${oldPDBs})
    file(REMOVE "${pdb}")
endforeach()

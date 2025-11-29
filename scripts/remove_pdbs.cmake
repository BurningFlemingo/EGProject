file(GLOB old_pdbs "${BINARY_DIR}/Game_*.pdb")
foreach(pdb ${old_pdbs})
    file(REMOVE "${pdb}")
endforeach()

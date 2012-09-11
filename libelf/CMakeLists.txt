CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9 FATAL_ERROR)
PROJECT(libelf)

# Disallow in-source build
IF("${libdwarf_SOURCE_DIR}" STREQUAL "${libdwarf_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "Project must be built out-of-source.  Remove files from the source tree that were created by this attempt:
  rm -rf CMakeFiles CMakeCache.txt
Then create a separate build directory and run CMake there.
  mkdir ../build
  cd ../build
  cmake ../
This approach keeps the source tree pristine.")
ENDIF()

# Path to path64 source dir
if(NOT MELODY_PATH)
    MESSAGE(FATAL_ERROR "Need to set PATH64_PATH to the path64 source tree.")
endif()

# Path to path64 build dir
if(NOT MELODY_BUILD_PATH)
    MESSAGE(FATAL_ERROR "Need to set PATH64_BUILD_PATH to the path64 build tree.")
endif()

INCLUDE_DIRECTORIES(BEFORE
  ${libdwarf_SOURCE_DIR}
  ${libdwarf_BINARY_DIR}
  ${PATH64_PATH}/src/libelf/lib
  ${PATH64_PATH}/src/include
  ${PATH64_BUILD_PATH}/src/include
)

# Force 64-bit on osol
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
SET(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m64")
SET(CMAKE_EXE_LINKER_FLAGS "-m64")
SET(CMAKE_MODULE_LINKER_FLAGS "-m64")
SET(CMAKE_SHARED_LINKER_FLAGS "-m64")

add_custom_command(
OUTPUT dwarf_pubnames.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pubnames.m4 > dwarf_pubnames.c
)
add_custom_command(
OUTPUT dwarf_pubtypes.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pubtypes.m4 > dwarf_pubtypes.c
)
add_custom_command(
OUTPUT dwarf_weaks.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_weaks.m4 > dwarf_weaks.c
)
add_custom_command(
OUTPUT dwarf_funcs.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_funcs.m4 > dwarf_funcs.c
)
add_custom_command(
OUTPUT dwarf_vars.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_vars.m4 > dwarf_vars.c
)
add_custom_command(
OUTPUT dwarf_types.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_types.m4 > dwarf_types.c
)

add_custom_command(
OUTPUT dwarf_pro_pubnames.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_pubnames.m4 > dwarf_pro_pubnames.c
)
add_custom_command(
OUTPUT dwarf_pro_weaks.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_weaks.m4 > dwarf_pro_weaks.c
)
add_custom_command(
OUTPUT dwarf_pro_funcs.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_funcs.m4 > dwarf_pro_funcs.c
)
add_custom_command(
OUTPUT dwarf_pro_vars.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_vars.m4 > dwarf_pro_vars.c
)
add_custom_command(
OUTPUT dwarf_pro_types.c
COMMAND m4 -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_nametbl.m4 ${CMAKE_CURRENT_SOURCE_DIR}/dwarf_pro_types.m4 > dwarf_pro_types.c
)



set(dwarf_sources 
	dwarf_abbrev.c
	dwarf_arange.c
	dwarf_attr.c
	dwarf_attrval.c
	dwarf_cu.c
	dwarf_dealloc.c
	dwarf_die.c
	dwarf_dump.c
	dwarf_errmsg.c
	dwarf_finish.c
	dwarf_form.c
	dwarf_frame.c
	dwarf_init.c
	dwarf_lineno.c
	dwarf_loclist.c
	dwarf_macinfo.c
	dwarf_pro_arange.c
	dwarf_pro_attr.c
	dwarf_pro_die.c
	dwarf_pro_expr.c
	dwarf_pro_finish.c
	dwarf_pro_frame.c
	dwarf_pro_init.c
	dwarf_pro_lineno.c
	dwarf_pro_macinfo.c
	dwarf_pro_reloc.c
	dwarf_pro_sections.c
	dwarf_ranges.c
	dwarf_reloc.c
	dwarf_seterror.c
	dwarf_str.c
	libdwarf.c
	libdwarf_abbrev.c
	libdwarf_arange.c
	libdwarf_attr.c
	libdwarf_die.c
	libdwarf_error.c
	libdwarf_elf_access.c
	libdwarf_elf_init.c
	libdwarf_frame.c
	libdwarf_info.c
	libdwarf_init.c
	libdwarf_lineno.c
	libdwarf_loc.c
	libdwarf_loclist.c
	libdwarf_macinfo.c
	libdwarf_nametbl.c
	libdwarf_ranges.c
	libdwarf_reloc.c
	libdwarf_rw.c
	libdwarf_sections.c
	libdwarf_str.c
	dwarf_funcs.c
	dwarf_pro_funcs.c
	dwarf_pro_pubnames.c
	dwarf_pro_types.c
	dwarf_pro_vars.c
	dwarf_pro_weaks.c
	dwarf_pubnames.c
	dwarf_pubtypes.c
	dwarf_types.c
	dwarf_vars.c
	dwarf_weaks.c
        )

add_library(dwarf STATIC ${dwarf_sources})
add_dependencies(dwarf path64_dir)

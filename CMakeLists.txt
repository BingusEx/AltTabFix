cmake_minimum_required(VERSION 3.22)
  
project(
	AltTabFix
	VERSION 1.2.1
	DESCRIPTION "TemplatePlugin"
	LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
add_compile_definitions(_DISABLE_EXTENDED_ALIGNED_STORAGE)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# #######################################################################################################################
# # Add CMake features
# #######################################################################################################################
include(XSEPlugin)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)

# #######################################################################################################################
# # Find dependencies
# #######################################################################################################################
find_package(xbyak CONFIG REQUIRED)
find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	${DETOURS_INCLUDE_DIRS}
	${CLIB_UTIL_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	xbyak::xbyak
	${DETOURS_LIBRARY}
)

# https://gitlab.kitware.com/cmake/cmake/-/issues/24922#note_1371990
if(MSVC_VERSION GREATER_EQUAL 1936 AND MSVC_IDE) # 17.6+
	# When using /std:c++latest, "Build ISO C++23 Standard Library Modules" defaults to "Yes".
	# Default to "No" instead.
	#
	# As of CMake 3.26.4, there isn't a way to control this property
	# (https://gitlab.kitware.com/cmake/cmake/-/issues/24922),
	# We'll use the MSBuild project system instead
	# (https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure)
	file(CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/Directory.Build.props" CONTENT [==[
<Project>
  <ItemDefinitionGroup>
    <ClCompile>
      <BuildStlModules>false</BuildStlModules>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
]==] @ONLY)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD

	COMMAND ${CMAKE_COMMAND} -E echo "Copying to: $ENV{GTSPLUGIN_COPY_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:${PROJECT_NAME}>"
		"$ENV{GTSPLUGIN_COPY_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
		"$ENV{GTSPLUGIN_COPY_DIR}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
	COMMENT "Copying DLL and PDB to GTSPLUGIN_COPY_DIR"
)


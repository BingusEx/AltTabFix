# #######################################################################################################################
# # Automatic deployment
# #######################################################################################################################

get_filename_component(BUILD_FOLDER "${CMAKE_BINARY_DIR}" NAME)
set(BUILD_NAME "${BUILD_FOLDER}")

if(PL_BUILD_DISTRIBUTION)

    message(STATUS "Automatic deployment build name: ${BUILD_NAME}")

    set(DEPLOY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/distribution/Package-${BUILD_NAME}")

    # Create the target deployment folder.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
      COMMENT "Creating deployment directory for ${BUILD_NAME}"
    )

    # Copy the main DLL.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
              $<TARGET_FILE:${PROJECT_NAME}>
              "${DEPLOY_DIR}/"
      COMMENT "Copying ${PROJECT_NAME} DLL to deployment directory"
    )

    # Copy the PDB if available
    if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo|Release")
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_PDB_FILE:${PROJECT_NAME}>
                "${DEPLOY_DIR}/"
        COMMENT "Copying ${PROJECT_NAME} PDB to deployment directory"
      )
    endif()
    
    # Copy Fonts directory
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/distribution/Bundle")
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/distribution/Bundle"
                "${DEPLOY_DIR}"
        COMMENT "Copying Bunle directory contents to deployment directory"
      )
    endif()

    # Clean output DLLs and PDBs
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/distribution")
      file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/distribution/Plugin-*/*.dll")
      file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/distribution/Plugin-*/*.pdb")

      if(OUTPUT_DLLS)
        set_property(TARGET ${PROJECT_NAME}
          APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
      endif()

      if(OUTPUT_PDBS)
        set_property(TARGET ${PROJECT_NAME}
          APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
      endif()
    endif()
endif()

# Copy to ENV_PLUGIN_COPY_DIR if defined
if(PL_DEPLOY_TO_FOLDER)
    if(DEFINED ENV{ENV_PLUGIN_COPY_DIR})
      message(STATUS "ENV_PLUGIN_COPY_DIR is set to: $ENV{ENV_PLUGIN_COPY_DIR}")

      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:${PROJECT_NAME}>
                "$ENV{ENV_PLUGIN_COPY_DIR}/"
        COMMENT "Copying ${PROJECT_NAME} DLL to ENV_PLUGIN_COPY_DIR"
      )

      if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo|Release")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  $<TARGET_PDB_FILE:${PROJECT_NAME}>
                  "$ENV{ENV_PLUGIN_COPY_DIR}/"
          COMMENT "Copying ${PROJECT_NAME} PDB to ENV_PLUGIN_COPY_DIR"
        )
      endif()
    endif()
 endif()
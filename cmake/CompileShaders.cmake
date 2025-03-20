function(compile_shaders)
  set(options)
  set(oneValueArgs SHADER_DIR OUTPUT_DIR TARGET_NAME)
  set(multiValueArgs SHADER_TYPES)
  cmake_parse_arguments(CS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  find_program(GLSLC glslangValidator)
  if(NOT GLSLC)
    message(FATAL_ERROR "glslangValidator not found")
  endif()

  file(MAKE_DIRECTORY ${CS_OUTPUT_DIR})

  set(SPV_FILES)

  foreach(type ${CS_SHADER_TYPES})
    file(GLOB shaders
      "${CS_SHADER_DIR}/*.${type}"
    )

    foreach(shader ${shaders})
      get_filename_component(SHADER_NAME ${shader} NAME)
      set(OUTPUT_SPV "${CS_OUTPUT_DIR}/${SHADER_NAME}.spv")

      add_custom_command(
        OUTPUT ${OUTPUT_SPV}
        COMMAND ${GLSLC} -V ${shader} -o ${OUTPUT_SPV}
        DEPENDS ${shader}
        COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
        VERBATIM
      )

      list(APPEND SPV_FILES ${OUTPUT_SPV})
    endforeach()
  endforeach()

  message(STATUS "SPV files to generate: ${SPV_FILES}")

  if(NOT SPV_FILES)
    message(WARNING "No shaders found. Check SHADER_DIR and SHADER_TYPES")
  endif()

  add_custom_target(${CS_TARGET_NAME} ALL
    DEPENDS ${SPV_FILES}
  )
endfunction()
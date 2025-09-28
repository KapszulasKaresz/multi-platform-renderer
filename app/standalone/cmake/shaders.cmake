function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/res/shaders)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)

    if(NOT DEFINED SLANGC_EXECUTABLE)
        find_program(SLANGC_EXECUTABLE slangc)
        if(NOT SLANGC_EXECUTABLE)
            message(FATAL_ERROR "slangc not found — please set SLANGC_EXECUTABLE manually (e.g., -DSLANGC_EXECUTABLE=/path/to/slangc)")
        else()
            message(STATUS "Found slangc: ${SLANGC_EXECUTABLE}")
        endif()
    endif()

    add_custom_command(
        OUTPUT ${SHADERS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )

    set(SPIRV_OUTPUTS)

    foreach(SHADER_FILE ${SHADER_SOURCES})
        get_filename_component(FILE_NAME ${SHADER_FILE} NAME_WE)
        set(OUTPUT_FILE ${SHADERS_DIR}/${FILE_NAME}.spv)

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${SLANGC_EXECUTABLE}
            ${SHADER_FILE}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            ${ENTRY_POINTS}
            -o ${OUTPUT_FILE}
            DEPENDS ${SHADERS_DIR} ${SHADER_FILE}
            COMMENT "Compiling Slang Shader ${SHADER_FILE} -> ${OUTPUT_FILE}"
            VERBATIM
        )

        list(APPEND SPIRV_OUTPUTS ${OUTPUT_FILE})
    endforeach()

    add_custom_target(${SHADER_COMPILE_STEP} DEPENDS ${SPIRV_OUTPUTS})
endfunction()

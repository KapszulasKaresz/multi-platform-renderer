function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/res/shaders)

    if(NOT DEFINED SLANGC_EXECUTABLE)
        find_program(SLANGC_EXECUTABLE slangc)
        if(NOT SLANGC_EXECUTABLE)
            message(FATAL_ERROR "slangc not found — please set SLANGC_EXECUTABLE manually (e.g., -DSLANGC_EXECUTABLE=/path/to/slangc)")
        else()
            message(STATUS "Found slangc: ${SLANGC_EXECUTABLE}")
        endif()
    endif()

    file(MAKE_DIRECTORY ${SHADERS_DIR})

    set(COMPILED_OUTPUTS)

    foreach(SHADER_FILE ${SHADER_SOURCES})
        get_filename_component(FILE_NAME ${SHADER_FILE} NAME_WE)

        set(SPV_OUTPUT ${SHADERS_DIR}/${FILE_NAME}.spv)
        set(DXIL_VERT_OUTPUT ${SHADERS_DIR}/${FILE_NAME}.vert.dxil)
        set(DXIL_FRAG_OUTPUT ${SHADERS_DIR}/${FILE_NAME}.frag.dxil)

        add_custom_command(
            OUTPUT ${SPV_OUTPUT}
            COMMAND ${SLANGC_EXECUTABLE}
            ${SHADER_FILE}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -entry vertMain
            -entry fragMain
            -o ${SPV_OUTPUT}
            DEPENDS ${SHADER_FILE}
            COMMENT "Compiling Slang -> SPIR-V: ${SHADER_FILE}"
            VERBATIM
        )

        add_custom_command(
            OUTPUT ${DXIL_VERT_OUTPUT}
            COMMAND ${SLANGC_EXECUTABLE}
            ${SHADER_FILE}
            -target dxil
            -profile sm_6_0
            -entry vertMain
            -o ${DXIL_VERT_OUTPUT}
            DEPENDS ${SHADER_FILE}
            COMMENT "Compiling Slang -> DXIL (Vertex): ${SHADER_FILE}"
            VERBATIM
        )

        add_custom_command(
            OUTPUT ${DXIL_FRAG_OUTPUT}
            COMMAND ${SLANGC_EXECUTABLE}
            ${SHADER_FILE}
            -target dxil
            -profile sm_6_0
            -entry fragMain
            -o ${DXIL_FRAG_OUTPUT}
            DEPENDS ${SHADER_FILE}
            COMMENT "Compiling Slang -> DXIL (Fragment): ${SHADER_FILE}"
            VERBATIM
        )

        list(APPEND COMPILED_OUTPUTS ${SPV_OUTPUT} ${DXIL_VERT_OUTPUT} ${DXIL_FRAG_OUTPUT})
    endforeach()

    add_custom_target(${TARGET} ALL DEPENDS ${COMPILED_OUTPUTS})
endfunction()

function(add_slang_dx_ray_tracing_target TARGET SHADER_FILE)
    if(NOT DEFINED SLANGC_EXECUTABLE)
        find_program(SLANGC_EXECUTABLE slangc)
        if(NOT SLANGC_EXECUTABLE)
            message(FATAL_ERROR "slangc not found — please set SLANGC_EXECUTABLE manually")
        endif()
    endif()

    set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/res/shaders/compiled)
    set(DXIL_OUTPUT ${SHADERS_DIR}/raytracing.dxil)
    file(MAKE_DIRECTORY ${SHADERS_DIR})

    add_custom_command(
        OUTPUT ${DXIL_OUTPUT}
        COMMAND ${SLANGC_EXECUTABLE}
        ${SHADER_FILE}
        -target dxil
        -profile lib_6_3
        -o ${DXIL_OUTPUT}
        DEPENDS ${SHADER_FILE}
            ${CMAKE_SOURCE_DIR}/res/shaders/raytracing.slang
            ${CMAKE_SOURCE_DIR}/res/shaders/raytracing_bindings_dx.slang
        COMMENT "Compiling Slang -> DXIL (Ray Tracing Library): ${SHADER_FILE}"
        VERBATIM
    )

    add_custom_target(${TARGET} ALL DEPENDS ${DXIL_OUTPUT})
endfunction()

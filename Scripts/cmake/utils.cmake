function(target_link_runtime target_name project_binary_directory)
    message(STATUS "Copy Immortal Runtime for target - ${target_name}")
    file(COPY ${IMMORTAL_ASSET_DIR} DESTINATION ${project_binary_directory})
    file(COPY ${IMMORTAL_RUNTIME} DESTINATION ${project_binary_directory})
    if (AgilitySDK_SHARED)
        file(COPY ${AgilitySDK_SHARED} DESTINATION ${project_binary_directory}/D3D12)
    endif()
endfunction()

function(immortal_target_link_runtime_dependency target_name)
    add_custom_command(TARGET ${target_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        copy ${IMMORTAL_RUNTIME} $<TARGET_FILE_DIR:${target_name}>
    COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${IMMORTAL_ASSET_DIR} $<TARGET_FILE_DIR:${target_name}>/Assets
    COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${AgilitySDK_SHARED} $<TARGET_FILE_DIR:${target_name}>/D3D12)
endfunction()

function(add_shader_library TARGET HLSL_FILES)
    add_library(${TARGET} INTERFACE)

    set(DXC_EXE "dxc")
    foreach(HLSL_FILE ${HLSL_FILES})
        get_filename_component(FILE_NAME "${HLSL_FILE}" NAME_WE)
        get_filename_component(FILE_PATH "${HLSL_FILE}" ABSOLUTE)
        set(DXIL_OUTPUT "${OUTPUT_DIR}/${FILE_NAME}.dxil")

        add_custom_target(
            compile_${FILE_NAME}_hlsl
            COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} -Wignored-attributes -T cs_6_0 -E main -Fo ${DXIL_OUTPUT} ${HLSL_FILE}"
            COMMAND "${DXC_EXE}"
                    "-Wignored-attributes"
                    "-T" "cs_6_0"
                    "-E" "main"
                    "-Fo" "${DXIL_OUTPUT}"
                    "${HLSL_FILE}"
            DEPENDS "${HLSL_FILE}"
            COMMENT "Compile ${FILE_NAME}.hlsl -> ${DXIL_OUTPUT}"
        )

        add_dependencies(${TARGET} compile_${FILE_NAME}_hlsl)
        set_property(TARGET compile_${FILE_NAME}_hlsl PROPERTY FOLDER "${TARGET}")
    endforeach()

    target_compile_definitions(${TARGET} INTERFACE
        HLSL_SHADERS_LIBRARY
    )
endfunction()

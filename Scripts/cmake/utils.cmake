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

# Output naming:
#   COMPUTE  -> <basename>.dxil                    (no extra suffix)
#   GRAPHICS -> <basename>_VS.dxil, <basename>_PS.dxil
#
# Copies all HLSL/*.hlsl to ${CMAKE_BINARY_DIR}/$<CONFIG>/Assets/Shaders/hlsl before dxc.
# Usage: add_shader_library(<target> HLSL_DIR <path> OUTPUT_DIR <path> COMPUTE "..." GRAPHICS "...")
function(add_shader_library TARGET)
    cmake_parse_arguments(ARG "" "HLSL_DIR;OUTPUT_DIR" "COMPUTE;GRAPHICS" ${ARGN})
    if(NOT ARG_HLSL_DIR)
        message(FATAL_ERROR "add_shader_library(${TARGET}): missing HLSL_DIR <directory with *.hlsl>")
    endif()
    if(NOT ARG_OUTPUT_DIR)
        message(FATAL_ERROR "add_shader_library(${TARGET}): missing OUTPUT_DIR (e.g. \${CMAKE_BINARY_DIR}/\$<CONFIG>/Assets/Shaders/hlsl)")
    endif()
    if(NOT DEFINED ARG_COMPUTE)
        set(ARG_COMPUTE "")
    endif()
    if(NOT DEFINED ARG_GRAPHICS)
        set(ARG_GRAPHICS "")
    endif()
    add_library(${TARGET} INTERFACE)
    set(DXC_EXE "dxc")
    get_filename_component(_hlsl_src_norm "${ARG_HLSL_DIR}" ABSOLUTE)
    set(_copy_tgt "${TARGET}_copy_hlsl_sources")
    file(GLOB _hlsl_glob "${_hlsl_src_norm}/*.hlsl")
    set(_copy_hlsl_cmds "")
    foreach(_hf ${_hlsl_glob})
        get_filename_component(_hname "${_hf}" NAME)
        list(APPEND _copy_hlsl_cmds COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_hf}" "${ARG_OUTPUT_DIR}/${_hname}")
    endforeach()
    add_custom_target(
        ${_copy_tgt}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT_DIR}"
        ${_copy_hlsl_cmds}
        COMMENT "Copy *.hlsl -> ${ARG_OUTPUT_DIR}"
    )
    set_property(TARGET ${_copy_tgt} PROPERTY FOLDER "${TARGET}")
    set(_shader_agg_deps "${_copy_tgt}")
    foreach(HLSL_FILE ${ARG_COMPUTE})
        get_filename_component(FILE_NAME "${HLSL_FILE}" NAME_WE)
        set(DXIL_OUTPUT "${ARG_OUTPUT_DIR}/${FILE_NAME}.dxil")

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
            COMMENT "Compile ${FILE_NAME}.hlsl (cs_6_0 main) -> ${DXIL_OUTPUT}"
        )
        list(APPEND _shader_agg_deps compile_${FILE_NAME}_hlsl)
        add_dependencies(compile_${FILE_NAME}_hlsl ${_copy_tgt})
        set_property(TARGET compile_${FILE_NAME}_hlsl PROPERTY FOLDER "${TARGET}")
    endforeach()
    foreach(HLSL_FILE ${ARG_GRAPHICS})
        get_filename_component(FILE_NAME "${HLSL_FILE}" NAME_WE)
        set(VS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_VS.dxil")
        set(PS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_PS.dxil")

        add_custom_target(
            compile_${FILE_NAME}_gfx_hlsl
            COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} ${FILE_NAME}.hlsl -> VS/PS dxil"
            COMMAND "${DXC_EXE}" "-Wignored-attributes" "-T" "vs_6_0" "-E" "VSMain" "-Fo" "${VS_OUT}" "${HLSL_FILE}"
            COMMAND "${DXC_EXE}" "-Wignored-attributes" "-T" "ps_6_0" "-E" "PSMain" "-Fo" "${PS_OUT}" "${HLSL_FILE}"
            DEPENDS "${HLSL_FILE}"
            COMMENT "Compile ${FILE_NAME}.hlsl (vs_6_0 VSMain, ps_6_0 PSMain)"
        )
        list(APPEND _shader_agg_deps compile_${FILE_NAME}_gfx_hlsl)
        add_dependencies(compile_${FILE_NAME}_gfx_hlsl ${_copy_tgt})
        set_property(TARGET compile_${FILE_NAME}_gfx_hlsl PROPERTY FOLDER "${TARGET}")
    endforeach()
    add_custom_target(
        ${TARGET}_build_shaders
        DEPENDS ${_shader_agg_deps}
        COMMENT "Shader pipeline for ${TARGET} (copy HLSL + compile DXIL)"
    )
    set_property(TARGET ${TARGET}_build_shaders PROPERTY FOLDER "${TARGET}")
    add_dependencies(${TARGET} ${TARGET}_build_shaders)
    target_compile_definitions(${TARGET} INTERFACE HLSL_SHADERS_LIBRARY)
endfunction()

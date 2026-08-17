function(target_link_runtime target_name project_binary_directory)
    message(STATUS "Copy Immortal Runtime for target - ${target_name}")
    file(COPY ${IMMORTAL_ASSET_DIR} DESTINATION ${project_binary_directory})
    file(COPY ${IMMORTAL_RUNTIME} DESTINATION ${project_binary_directory})
    if (AgilitySDK_SHARED)
        file(COPY ${AgilitySDK_SHARED} DESTINATION ${project_binary_directory}/D3D12)
    endif()
endfunction()

function(immortal_target_link_runtime_dependency target_name)
	set(_dll_copy_cmds "")
	set(_runtime_path_entries "")
	foreach(_rt ${IMMORTAL_RUNTIME})
		list(APPEND _dll_copy_cmds
			COMMAND ${CMAKE_COMMAND}
				"-DSRC=${_rt}"
				"-DDST=$<TARGET_FILE_DIR:${target_name}>"
				-P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/copy_if_exists.cmake")
		if(_rt MATCHES "^\\$<\\$<CONFIG:([^>]+)>:(.+)>$")
			set(_rt_config "${CMAKE_MATCH_1}")
			set(_rt_path "${CMAKE_MATCH_2}")
			get_filename_component(_rt_dir "${_rt_path}" DIRECTORY)
			list(APPEND _runtime_path_entries "$<$<CONFIG:${_rt_config}>:${_rt_dir}>")
		elseif(NOT _rt STREQUAL "")
			get_filename_component(_rt_dir "${_rt}" DIRECTORY)
			list(APPEND _runtime_path_entries "${_rt_dir}")
		endif()
	endforeach()
	set(_runtime_copy_cmds
		${_dll_copy_cmds}
		COMMAND ${CMAKE_COMMAND} -E
			copy_directory ${IMMORTAL_ASSET_DIR} $<TARGET_FILE_DIR:${target_name}>/Assets)
	if(AgilitySDK_SHARED)
		file(GLOB _agility_runtime_files "${AgilitySDK_SHARED}/*")
		foreach(_agility_rt ${_agility_runtime_files})
			list(APPEND _runtime_copy_cmds
				COMMAND ${CMAKE_COMMAND}
					"-DSRC=${_agility_rt}"
					"-DDST=$<TARGET_FILE_DIR:${target_name}>/D3D12"
					-P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/copy_if_exists.cmake")
		endforeach()
		list(APPEND _runtime_path_entries "${AgilitySDK_SHARED}")
	endif()
	add_custom_command(TARGET ${target_name} PRE_BUILD
		${_runtime_copy_cmds})
	add_custom_command(TARGET ${target_name} POST_BUILD
		${_runtime_copy_cmds})
	if(_runtime_path_entries)
		list(REMOVE_DUPLICATES _runtime_path_entries)
		string(JOIN "\\;" _runtime_path "${_runtime_path_entries}")
		set_property(TARGET ${target_name} PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=${_runtime_path};%PATH%")
	endif()
endfunction()

option(
    IMMORTAL_STAGE_ALL_HLSL_SOURCES
    "Stage every HLSL/HLSLI source for non-D3D12 runtime shader compilation"
    OFF
)

function(add_shader_library TARGET)
    cmake_parse_arguments(ARG "" "HLSL_DIR;OUTPUT_DIR" "COMPUTE;GRAPHICS;GRAPHICS_MESH;RUNTIME_HLSL" ${ARGN})
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
    if(NOT DEFINED ARG_GRAPHICS_MESH)
        set(ARG_GRAPHICS_MESH "")
    endif()
    if(NOT DEFINED ARG_RUNTIME_HLSL)
        set(ARG_RUNTIME_HLSL "")
    endif()
    add_library(${TARGET} INTERFACE)
    # Prefer a resolved dxc so CMake actually emits .dxil (PATH may omit Vulkan SDK / Windows Kit).
    find_program(
        IMMORTAL_DXC_EXECUTABLE
        NAMES dxc dxc.exe
        HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VULKAN_SDK}/Bin32"
    )
    if(IMMORTAL_DXC_EXECUTABLE)
        set(DXC_EXE "${IMMORTAL_DXC_EXECUTABLE}")
        message(STATUS "${TARGET}: HLSL -> DXIL using `${DXC_EXE}`")
    else()
        set(DXC_EXE "dxc")
        message(WARNING "${TARGET}: `dxc` not found via CMAKE_PROGRAM_PATH/VULKAN_SDK; falling back to `dxc` on PATH. Install Vulkan SDK or add dxc to PATH to generate .dxil.")
    endif()
    get_filename_component(_hlsl_src_norm "${ARG_HLSL_DIR}" ABSOLUTE)
    set(_copy_tgt "${TARGET}_copy_hlsl_sources")
    file(GLOB _hlsl_glob
        "${_hlsl_src_norm}/*.hlsl"
        "${_hlsl_src_norm}/*.hlsli"
    )
    set(_staged_hlsl_files "")
    if(IMMORTAL_STAGE_ALL_HLSL_SOURCES)
        set(_staged_hlsl_files ${_hlsl_glob})
        set(_stage_hlsl_comment "Stage all HLSL sources -> ${ARG_OUTPUT_DIR}")
    else()
        foreach(_runtime_hlsl ${ARG_RUNTIME_HLSL})
            if(IS_ABSOLUTE "${_runtime_hlsl}")
                set(_runtime_hlsl_abs "${_runtime_hlsl}")
            else()
                set(_runtime_hlsl_abs "${_hlsl_src_norm}/${_runtime_hlsl}")
            endif()
            get_filename_component(_runtime_hlsl_abs "${_runtime_hlsl_abs}" ABSOLUTE)
            if(NOT EXISTS "${_runtime_hlsl_abs}")
                message(FATAL_ERROR "add_shader_library(${TARGET}): runtime HLSL does not exist: ${_runtime_hlsl_abs}")
            endif()
            list(APPEND _staged_hlsl_files "${_runtime_hlsl_abs}")
        endforeach()
        list(REMOVE_DUPLICATES _staged_hlsl_files)
        set(_stage_hlsl_comment "Stage runtime-only HLSL sources -> ${ARG_OUTPUT_DIR}")
    endif()

    set(_copy_hlsl_cmds "")
    foreach(_hf ${_hlsl_glob})
        get_filename_component(_hname "${_hf}" NAME)
        list(FIND _staged_hlsl_files "${_hf}" _staged_hlsl_index)
        if(_staged_hlsl_index LESS 0)
            list(APPEND _copy_hlsl_cmds COMMAND ${CMAKE_COMMAND} -E remove -f "${ARG_OUTPUT_DIR}/${_hname}")
        endif()
    endforeach()
    foreach(_hf ${_staged_hlsl_files})
        get_filename_component(_hname "${_hf}" NAME)
        list(APPEND _copy_hlsl_cmds COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_hf}" "${ARG_OUTPUT_DIR}/${_hname}")
    endforeach()
    add_custom_target(
        ${_copy_tgt}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT_DIR}"
        ${_copy_hlsl_cmds}
        COMMENT "${_stage_hlsl_comment}"
    )
    set_property(TARGET ${_copy_tgt} PROPERTY FOLDER "${TARGET}")
    set(_shader_agg_deps "${_copy_tgt}")
    set(_shader_outputs "")
    foreach(_compute_item ${ARG_COMPUTE})
        string(REPLACE "|" ";" _compute_segments "${_compute_item}")
        list(LENGTH _compute_segments _compute_segment_count)
        list(GET _compute_segments 0 HLSL_FILE)
        if(_compute_segment_count GREATER 1)
            list(GET _compute_segments 1 _compute_entry)
        else()
            set(_compute_entry "main")
        endif()

        get_filename_component(FILE_NAME "${HLSL_FILE}" NAME_WE)
        if(_compute_segment_count GREATER 2)
            list(GET _compute_segments 2 _compute_output_name)
        elseif(_compute_entry STREQUAL "main")
            set(_compute_output_name "${FILE_NAME}")
        else()
            set(_compute_output_name "${FILE_NAME}_${_compute_entry}")
        endif()
        string(MAKE_C_IDENTIFIER "${_compute_output_name}" _compute_target_name)
        set(DXIL_OUTPUT "${ARG_OUTPUT_DIR}/${_compute_output_name}.dxil")
        list(APPEND _shader_outputs "${DXIL_OUTPUT}")

        add_custom_target(
            compile_${_compute_target_name}_hlsl
            COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} -Wignored-attributes -T cs_6_0 -E ${_compute_entry} -Fo ${DXIL_OUTPUT} ${HLSL_FILE}"
            COMMAND "${DXC_EXE}"
                    "-Wignored-attributes"
                    "-T" "cs_6_0"
                    "-E" "${_compute_entry}"
                    "-Fo" "${DXIL_OUTPUT}"
                    "${HLSL_FILE}"
            DEPENDS "${HLSL_FILE}"
            COMMENT "Compile ${FILE_NAME}.hlsl (cs_6_0 ${_compute_entry}) -> ${DXIL_OUTPUT}"
        )
        list(APPEND _shader_agg_deps compile_${_compute_target_name}_hlsl)
        add_dependencies(compile_${_compute_target_name}_hlsl ${_copy_tgt})
        set_property(TARGET compile_${_compute_target_name}_hlsl PROPERTY FOLDER "${TARGET}")
    endforeach()
    set(_gfx_vs_built "")
    foreach(_gfx_item ${ARG_GRAPHICS})
        string(REPLACE "|" ";" _gfx_seg "${_gfx_item}")
        list(LENGTH _gfx_seg _gfx_seg_len)
        if(_gfx_seg_len GREATER 1)
            list(GET _gfx_seg 0 HLSL_FILE)
            list(GET _gfx_seg 1 _ps_entry)
        else()
            set(HLSL_FILE "${_gfx_item}")
            set(_ps_entry "PSMain")
        endif()
        set(_gfx_dxc_def_flags "")
        if(_gfx_seg_len GREATER 2)
            list(GET _gfx_seg 2 _gfx_dxc_define_pair)
            list(APPEND _gfx_dxc_def_flags "-D${_gfx_dxc_define_pair}")
        endif()
        get_filename_component(_gfx_hlsl_abs "${HLSL_FILE}" ABSOLUTE)
        get_filename_component(FILE_NAME "${_gfx_hlsl_abs}" NAME_WE)
        set(VS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_VS.dxil")
        if(_ps_entry STREQUAL "PSMain")
            set(PS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_PS.dxil")
        else()
            set(PS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_${_ps_entry}.dxil")
        endif()
        string(MAKE_C_IDENTIFIER "${_ps_entry}" _ps_id)
        set(_gfx_vs_target "compile_${FILE_NAME}_gfx_vs_hlsl")
        list(FIND _gfx_vs_built "${_gfx_hlsl_abs}" _gfx_vs_idx)
        if(_gfx_vs_idx LESS 0)
            list(APPEND _gfx_vs_built "${_gfx_hlsl_abs}")
            list(APPEND _shader_outputs "${VS_OUT}")
            add_custom_target(
                ${_gfx_vs_target}
                COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} ${FILE_NAME}.hlsl -> VS dxil (vs_6_0 VSMain)"
                COMMAND "${DXC_EXE}" "-Wignored-attributes" ${_gfx_dxc_def_flags} "-T" "vs_6_0" "-E" "VSMain" "-Fo" "${VS_OUT}" "${_gfx_hlsl_abs}"
                DEPENDS "${_gfx_hlsl_abs}"
                COMMENT "Compile ${FILE_NAME}.hlsl (vs_6_0 VSMain) -> ${VS_OUT}"
            )
            list(APPEND _shader_agg_deps ${_gfx_vs_target})
            add_dependencies(${_gfx_vs_target} ${_copy_tgt})
            set_property(TARGET ${_gfx_vs_target} PROPERTY FOLDER "${TARGET}")
        endif()
        list(APPEND _shader_outputs "${PS_OUT}")
        add_custom_target(
            compile_${FILE_NAME}_gfx_${_ps_id}_hlsl
            COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} ${FILE_NAME}.hlsl -> PS dxil (ps_6_0 ${_ps_entry})"
            COMMAND "${DXC_EXE}" "-Wignored-attributes" ${_gfx_dxc_def_flags} "-T" "ps_6_0" "-E" "${_ps_entry}" "-Fo" "${PS_OUT}" "${_gfx_hlsl_abs}"
            DEPENDS "${_gfx_hlsl_abs}"
            COMMENT "Compile ${FILE_NAME}.hlsl (ps_6_0 ${_ps_entry}) -> ${PS_OUT}"
        )
        list(APPEND _shader_agg_deps compile_${FILE_NAME}_gfx_${_ps_id}_hlsl)
        add_dependencies(compile_${FILE_NAME}_gfx_${_ps_id}_hlsl ${_copy_tgt})
        add_dependencies(compile_${FILE_NAME}_gfx_${_ps_id}_hlsl ${_gfx_vs_target})
        set_property(TARGET compile_${FILE_NAME}_gfx_${_ps_id}_hlsl PROPERTY FOLDER "${TARGET}")
    endforeach()

    # Mesh + pixel (same source file; MS compiled once per file, PS per GRAPHICS_MESH line).
    set(_mesh_ms_built "")
    foreach(_mesh_item ${ARG_GRAPHICS_MESH})
        string(REPLACE "|" ";" _mesh_seg "${_mesh_item}")
        list(LENGTH _mesh_seg _mesh_seg_len)
        if(_mesh_seg_len GREATER 1)
            list(GET _mesh_seg 0 HLSL_FILE)
            list(GET _mesh_seg 1 _ps_entry)
        else()
            set(HLSL_FILE "${_mesh_item}")
            set(_ps_entry "PSMain")
        endif()
        get_filename_component(_mesh_hlsl_abs "${HLSL_FILE}" ABSOLUTE)
        get_filename_component(FILE_NAME "${_mesh_hlsl_abs}" NAME_WE)
        set(MS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_MS.dxil")
        if(_ps_entry STREQUAL "PSMain")
            set(PS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_PS.dxil")
        else()
            set(PS_OUT "${ARG_OUTPUT_DIR}/${FILE_NAME}_${_ps_entry}.dxil")
        endif()

        list(FIND _mesh_ms_built "${_mesh_hlsl_abs}" _ms_idx)
        if(_ms_idx LESS 0)
            list(APPEND _mesh_ms_built "${_mesh_hlsl_abs}")
            list(APPEND _shader_outputs "${MS_OUT}")
            add_custom_target(
                compile_${FILE_NAME}_mesh_ms_hlsl
                COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} ${FILE_NAME}.hlsl -> MS dxil (ms_6_5 MSMain)"
                COMMAND "${DXC_EXE}" "-Wignored-attributes" "-T" "ms_6_5" "-E" "MSMain" "-Fo" "${MS_OUT}" "${_mesh_hlsl_abs}"
                DEPENDS "${_mesh_hlsl_abs}"
                COMMENT "Compile ${FILE_NAME}.hlsl (ms_6_5 MSMain) -> ${MS_OUT}"
            )
            list(APPEND _shader_agg_deps compile_${FILE_NAME}_mesh_ms_hlsl)
            add_dependencies(compile_${FILE_NAME}_mesh_ms_hlsl ${_copy_tgt})
            set_property(TARGET compile_${FILE_NAME}_mesh_ms_hlsl PROPERTY FOLDER "${TARGET}")
        endif()

        string(MAKE_C_IDENTIFIER "${_ps_entry}" _ps_id)
        list(APPEND _shader_outputs "${PS_OUT}")
        add_custom_target(
            compile_${FILE_NAME}_mesh_ps_${_ps_id}_hlsl
            COMMAND ${CMAKE_COMMAND} -E echo "${DXC_EXE} ${FILE_NAME}.hlsl -> PS dxil (ps_6_1 ${_ps_entry})"
            COMMAND "${DXC_EXE}" "-Wignored-attributes" "-T" "ps_6_1" "-E" "${_ps_entry}" "-Fo" "${PS_OUT}" "${_mesh_hlsl_abs}"
            DEPENDS "${_mesh_hlsl_abs}"
            COMMENT "Compile ${FILE_NAME}.hlsl (ps_6_1 ${_ps_entry}) -> ${PS_OUT}"
        )
        list(APPEND _shader_agg_deps compile_${FILE_NAME}_mesh_ps_${_ps_id}_hlsl)
        add_dependencies(compile_${FILE_NAME}_mesh_ps_${_ps_id}_hlsl ${_copy_tgt})
        add_dependencies(compile_${FILE_NAME}_mesh_ps_${_ps_id}_hlsl compile_${FILE_NAME}_mesh_ms_hlsl)
        set_property(TARGET compile_${FILE_NAME}_mesh_ps_${_ps_id}_hlsl PROPERTY FOLDER "${TARGET}")
    endforeach()

    list(REMOVE_DUPLICATES _shader_outputs)
    set(_shader_validation_script "${CMAKE_CURRENT_BINARY_DIR}/validate_${TARGET}_shaders_$<CONFIG>.cmake")
    set(_shader_validation_content "set(_shader_outputs\n")
    foreach(_shader_output ${_shader_outputs})
        string(APPEND _shader_validation_content "    [==[${_shader_output}]==]\n")
    endforeach()
    string(APPEND _shader_validation_content [=[
)
set(_missing_outputs)
foreach(_shader_output IN LISTS _shader_outputs)
    if(NOT EXISTS "${_shader_output}")
        list(APPEND _missing_outputs "${_shader_output}")
        continue()
    endif()
    file(SIZE "${_shader_output}" _shader_size)
    if(_shader_size EQUAL 0)
        list(APPEND _missing_outputs "${_shader_output} (empty)")
    endif()
endforeach()
if(_missing_outputs)
    list(JOIN _missing_outputs "\n  " _missing_text)
    message(FATAL_ERROR "Shader package is incomplete. Missing or empty DXIL files:\n  ${_missing_text}")
endif()
list(LENGTH _shader_outputs _shader_count)
message(STATUS "Validated ${_shader_count} DXIL shader assets")
]=])
    file(GENERATE
        OUTPUT "${_shader_validation_script}"
        CONTENT "${_shader_validation_content}")

    add_custom_target(
        ${TARGET}_build_shaders
        COMMAND ${CMAKE_COMMAND} -P "${_shader_validation_script}"
        DEPENDS ${_shader_agg_deps}
        COMMENT "Shader pipeline for ${TARGET} (copy HLSL + compile and validate DXIL)"
        VERBATIM
    )
    set_property(TARGET ${TARGET}_build_shaders PROPERTY FOLDER "${TARGET}")
    add_dependencies(${TARGET} ${TARGET}_build_shaders)
    target_compile_definitions(${TARGET} INTERFACE HLSL_SHADERS_LIBRARY)
endfunction()

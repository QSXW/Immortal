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

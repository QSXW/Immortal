# copy_if_exists.cmake -- copy SRC to DST, silently skip when SRC is empty
# (generator expressions for inactive configurations expand to "").
if(NOT "${SRC}" STREQUAL "" AND EXISTS "${SRC}")
    file(MAKE_DIRECTORY "${DST}")
    get_filename_component(_fname "${SRC}" NAME)
    file(COPY_FILE "${SRC}" "${DST}/${_fname}" ONLY_IF_DIFFERENT)
endif()

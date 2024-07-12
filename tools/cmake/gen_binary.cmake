
# set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET>.elf <LINK_LIBRARIES>")
# set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET>.elf <LINK_LIBRARIES>")

# add_custom_command(TARGET ${PROJECT_ID} POST_BUILD
#     COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${CMAKE_BINARY_DIR}/${PROJECT_ID}.elf ${CMAKE_BINARY_DIR}/${PROJECT_ID}.bin
#     DEPENDS ${PROJECT_ID}
#     COMMENT "-- Generating binary file ...")

# variable #{g_dynamic_libs} have dependency dynamic libs and compiled dynamic libs(register component and assigned DYNAMIC or SHARED flag)

string(TOLOWER ${BUILD_TYPE} build_type)

set(cp_dist_cmd COMMAND mkdir -p ${PROJECT_DIST_DIR}/${PROJECT_ID}_${build_type} && cp ${CMAKE_BINARY_DIR}/${PROJECT_ID} ${PROJECT_DIST_DIR}/${PROJECT_ID}_${build_type}/)
if(g_dynamic_libs)
    set(cp_command COMMAND cp ${g_dynamic_libs} ${CMAKE_BINARY_DIR}/dl_lib/)
    set(cp_dl_to_dist_cmd COMMAND mkdir -p ${PROJECT_DIST_DIR}/${PROJECT_ID}_${build_type}/dl_lib && cp ${g_dynamic_libs} ${PROJECT_DIST_DIR}/${PROJECT_ID}_${build_type}/dl_lib)
endif()

if(${BUILD_TYPE} STREQUAL "Release")
    set(strip_cmd COMMAND ${CMAKE_STRIP} ${CMAKE_BINARY_DIR}/${PROJECT_ID})
    # add dl lib to strip_cmd
    if(g_dynamic_libs)
        set(strip_cmd COMMAND ${CMAKE_STRIP} ${CMAKE_BINARY_DIR}/${PROJECT_ID} ${g_dynamic_libs})
    endif()
endif()

set(cp_assets_cmd COMMAND python ${SDK_PATH}/tools/cmake/copy_assets.py ${PROJECT_PATH} ${PROJECT_DIST_DIR}/${PROJECT_ID}_${build_type})

add_custom_command(TARGET ${PROJECT_ID} POST_BUILD
    ${strip_cmd}
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dl_lib
    ${cp_command}
    ${cp_dist_cmd}
    ${cp_dl_to_dist_cmd}
    ${cp_assets_cmd}
    DEPENDS ${PROJECT_ID}
    COMMENT "-- copy dynamic libs to build/dl_lib dir ...")


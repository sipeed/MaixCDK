#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license Apache 2.0
#


# Convert to cmake path(for Windows)
file(TO_CMAKE_PATH "${SDK_PATH}" SDK_PATH)

get_filename_component(parent_dir ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(current_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
get_filename_component(parent_dir_name ${parent_dir} NAME)

#  global variables
set(g_dynamic_libs "" CACHE INTERNAL "g_dynamic_libs")
set(g_dist_ignore_libs "" CACHE INTERNAL "g_dist_ignore_libs")
set(g_link_search_path "" CACHE INTERNAL "g_link_search_path")
set(g_includes "" CACHE INTERNAL "g_includes")
set(g_srcs "" CACHE INTERNAL "g_srcs")
# set(g_files_download_info "" CACHE INTERNAL "g_files_download_info")
set(g_enabled_components "" CACHE INTERNAL "g_enabled_components")

# Set project dir, so just projec can include this cmake file!!!
set(PROJECT_SOURCE_DIR ${parent_dir})
set(PROJECT_PATH       ${PROJECT_SOURCE_DIR})
set(PROJECT_BINARY_DIR "${parent_dir}/build")
set(PROJECT_DIST_DIR   "${parent_dir}/dist")
set(DL_PKGS_PATH       "${SDK_PATH}/dl/pkgs")
set(DL_EXTRACTED_PATH    "${SDK_PATH}/dl/extracted")
message(STATUS "PLATFORM: ${PLATFORM}")
message(STATUS "SDK_PATH: ${SDK_PATH}")
message(STATUS "PROJECT_PATH: ${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message(STATUS "PROJECT_DIST_DIR: ${PROJECT_DIST_DIR}")
message(STATUS "DL_PKGS_PATH: ${DL_PKGS_PATH}")
message(STATUS "DL_EXTRACTED_PATH: ${DL_EXTRACTED_PATH}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE} (${BUILD_TYPE})")
if(TOOLCHAIN_ID)
    message(STATUS "TOOLCHAIN_ID: ${TOOLCHAIN_ID}")
endif()

# ignore some cmake warning
set(DEFAULT_CONFIG_FILE ${DEFAULT_CONFIG_FILE})

include(${SDK_PATH}/tools/cmake/tools.cmake)

function(register_component)
    get_filename_component(component_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
    get_filename_component(component_name ${component_dir} NAME)
    message(STATUS "[register component: ${component_name} ], path:${component_dir}")

    # Get params: DYNAMIC/SHARED
    foreach(name ${ARGN})
        string(TOUPPER ${name} name)
        if(${name} STREQUAL "DYNAMIC" OR ${name} STREQUAL "SHARED")
            set(to_dynamic_lib true)
        endif()
    endforeach()
    if(to_dynamic_lib)
        message("-- component ${component_name} will compiled to dynamic lib")
        # Add dynamic file path to g_dynamic_libs variable
        set(dynamic_libs ${g_dynamic_libs})
        list(APPEND dynamic_libs "${component_name}+${PROJECT_BINARY_DIR}/${component_name}/lib${component_name}${DL_EXT}")
        set(g_dynamic_libs ${dynamic_libs}  CACHE INTERNAL "g_dynamic_libs")
    else()
        message("-- component ${component_name} will compiled to static lib")
    endif()

    # Add src to lib
    if(ADD_SRCS)
        if(to_dynamic_lib)
            add_library(${component_name} SHARED ${ADD_SRCS})
        else()
            add_library(${component_name} STATIC ${ADD_SRCS})
        endif()
        set(include_type PUBLIC)
        set(_srcs ${g_srcs})
        foreach(src ${ADD_SRCS})
            get_filename_component(abs_dir ${src} ABSOLUTE BASE_DIR ${component_dir})
            list(APPEND _srcs "${component_name}+${abs_dir}")
        endforeach()
        set(g_srcs ${_srcs} CACHE INTERNAL "g_srcs")
    else()
        if(to_dynamic_lib)
            add_library(${component_name} SHARED)
            set(include_type PUBLIC)
        else()
            add_library(${component_name} INTERFACE)
            set(include_type INTERFACE)
        endif()
    endif()

    # Add include
    foreach(include_dir ${ADD_INCLUDE})
        get_filename_component(abs_dir ${include_dir} ABSOLUTE BASE_DIR ${component_dir})
        if(NOT IS_DIRECTORY ${abs_dir})
            get_property(is_generated SOURCE ${include_dir} PROPERTY GENERATED)
            # if(NOT is_generated)
            #     message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: ${include_dir} not found!")
            # endif()
        endif()
        target_include_directories(${component_name} ${include_type} ${abs_dir})
        set(_includes ${g_includes})
        list(APPEND _includes "${component_name}+${abs_dir}")
        list(REMOVE_DUPLICATES _includes)
        set(g_includes ${_includes} CACHE INTERNAL "g_includes")
    endforeach()

    # Add private include
    foreach(include_dir ${ADD_PRIVATE_INCLUDE})
        if(${include_type} STREQUAL INTERFACE)
            message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: ADD_PRIVATE_INCLUDE set but no source file！")
        endif()
        get_filename_component(abs_dir ${include_dir} ABSOLUTE BASE_DIR ${component_dir})
        if(NOT IS_DIRECTORY ${abs_dir})
            message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: ${include_dir} not found!")
        endif()
        target_include_directories(${component_name} PRIVATE ${abs_dir})
    endforeach()

    # Add blobal config include
    if(${include_type} STREQUAL INTERFACE)
        target_include_directories(${component_name} INTERFACE ${global_config_dir})
    else()
        target_include_directories(${component_name} PUBLIC ${global_config_dir})
    endif()

    # Add definitions public
    foreach(difinition ${ADD_DEFINITIONS})
        if(${include_type} STREQUAL INTERFACE)
            target_compile_options(${component_name} INTERFACE ${difinition})
            target_link_options(${component_name} INTERFACE ${difinition})
        else()
            target_compile_options(${component_name} PUBLIC ${difinition})
            target_link_options(${component_name} PUBLIC ${difinition})
        endif()
    endforeach()

    # Add definitions private
    foreach(difinition ${ADD_DEFINITIONS_PRIVATE})
        target_compile_options(${component_name} PRIVATE ${difinition})
        target_link_options(${component_name} PRIVATE ${difinition})
    endforeach()

    # Add lib search path
    if(ADD_LINK_SEARCH_PATH)
        foreach(path ${ADD_LINK_SEARCH_PATH})
            if(NOT EXISTS "${path}")
                if(IS_ABSOLUTE ${lib})
                    set(lib_full ${lib})
                else()
                    prepend(lib_full "${component_dir}/" ${path})
                endif()
                if(NOT EXISTS "${lib_full}")
                    message(FATAL_ERROR "Can not find ${path} or ${lib_full}")
                endif()
                set(path ${lib_full})
            endif()
            get_filename_component(abs_dir ${path} ABSOLUTE)
            if(EXISTS "${abs_dir}")
                set(link_search_path ${g_link_search_path})
                list(APPEND link_search_path "${component_name}+${abs_dir}")
                # target_link_directories(${component_name} PUBLIC ${link_search_path}) # this will fail add -L -Wl,-rpath flag for some .so
                list(REMOVE_DUPLICATES link_search_path)
                set(g_link_search_path ${link_search_path}  CACHE INTERNAL "g_link_search_path")
            endif()
        endforeach()
    endif()

    # Add static lib
    if(ADD_STATIC_LIB)
        foreach(lib ${ADD_STATIC_LIB})
            if(NOT EXISTS "${lib}")
                if(IS_ABSOLUTE ${lib})
                    set(lib_full ${lib})
                else()
                    prepend(lib_full "${component_dir}/" ${lib})
                endif()
                if(NOT EXISTS "${lib_full}")
                    get_property(is_generated SOURCE ${lib} PROPERTY GENERATED)
                    if(NOT is_generated)
                        message(FATAL_ERROR "Can not find ${lib} or ${lib_full}")
                    endif()
                else()
                    set(lib ${lib_full})
                endif()
            endif()
            target_link_libraries(${component_name} ${include_type} ${lib})
        endforeach()
    endif()
    # Add dynamic lib
    if(ADD_DYNAMIC_LIB)
        set(dynamic_libs ${g_dynamic_libs})
        foreach(lib ${ADD_DYNAMIC_LIB})
            if(NOT EXISTS "${lib}")
                if(IS_ABSOLUTE ${lib})
                    set(lib_full ${lib})
                else()
                    prepend(lib_full "${component_dir}/" ${lib})
                endif()
                if(NOT EXISTS "${lib_full}")
                    get_property(is_generated SOURCE ${lib} PROPERTY GENERATED)
                    if(NOT is_generated)
                        message(FATAL_ERROR "Can not find ${lib} or ${lib_full}")
                    endif()
                else()
                    set(lib ${lib_full})
                endif()
            endif()
            get_filename_component(lib ${lib} ABSOLUTE)
            list(APPEND dynamic_libs "${component_name}+${lib}")
            get_filename_component(lib_dir ${lib} DIRECTORY)
            get_filename_component(lib_name ${lib} NAME)
            target_link_libraries(${component_name} ${include_type} -L${lib_dir} ${lib_name})
        endforeach()
        list(REMOVE_DUPLICATES dynamic_libs)
        set(g_dynamic_libs ${dynamic_libs}  CACHE INTERNAL "g_dynamic_libs")
    endif()

    # ignore libs
    if(ADD_DIST_LIB_IGNORE)
        set(dynamic_libs ${g_dist_ignore_libs})
        foreach(lib ${ADD_DIST_LIB_IGNORE})
            get_filename_component(lib ${lib} ABSOLUTE)
            list(APPEND dynamic_libs "${component_name}+${lib}")
        endforeach()
        list(REMOVE_DUPLICATES dynamic_libs)
        set(g_dist_ignore_libs ${dynamic_libs}  CACHE INTERNAL "g_dist_ignore_libs")
    endif()

    # Add requirements
    # get requirements from component.py
    set(cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/build.py get_requirements ${component_name} ${component_dir} ${SDK_PATH}/components ${SDK_PATH}/components/3rd_party ${SDK_PATH}/components/ext_devs ${MAIXCDK_EXTRA_COMPONENTS_PATH} ${PY_PKG_COMPONENTS_PATH} ${PY_USR_PKG_COMPONENTS_PATH} ${PROJECT_SOURCE_DIR}/../components ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/components)
    execute_process(${cmd} RESULT_VARIABLE cmd_res OUTPUT_VARIABLE component_requires)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Get valid components failed")
    endif()
    if(component_requires)
        message("-- component ${component_name} depend on components from component.py: ${component_requires}")
        list(APPEND ADD_REQUIREMENTS ${component_requires})
    endif()
    target_link_libraries(${component_name} ${include_type} ${ADD_REQUIREMENTS})
    set(save_req_cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/components_depends.py
                            append
                            ${CMAKE_BINARY_DIR}/config/components_depends.json
                            ${component_name} ${ADD_REQUIREMENTS}
    )
    execute_process(${save_req_cmd} RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Save requirements info failed")
    endif()

    # Add file depends
    if(ADD_FILE_DEPENDS)
        add_custom_target(${component_name}_file_depends DEPENDS ${ADD_FILE_DEPENDS})
        add_dependencies(${component_name} ${component_name}_file_depends)
    endif()

    # # Add files to download list to g_files_download_info
    # set(files ${g_files_download_info})
    # if(ADD_FILE_DOWNLOADS)
    #     foreach(file ${ADD_FILE_DOWNLOADS})
    #         string(REPLACE "\n" " " file ${file})
    #         string(REPLACE "\r" " " file ${file})
    #         list(APPEND files "${component_name}+${file}")
    #     endforeach()
    # endif()
    # # append file to global variable g_files_download_info
    # set(g_files_download_info ${files} CACHE INTERNAL "g_files_download_info")
endfunction()

function(is_path_component ret param_path)
    get_filename_component(abs_dir ${param_path} ABSOLUTE)

    if(NOT IS_DIRECTORY "${abs_dir}")
        set(${ret} 0 PARENT_SCOPE)
        return()
    endif()

    get_filename_component(base_dir ${abs_dir} NAME)
    string(SUBSTRING "${base_dir}" 0 1 first_char)

    if(first_char STREQUAL ".")
        set(${ret} 0 PARENT_SCOPE)
        return()
    endif()
    if(NOT EXISTS "${abs_dir}/CMakeLists.txt")
        set(${ret} 0 PARENT_SCOPE)
        return()
    endif()

    # check if register_component in CMakeLists.txt
    file(READ "${abs_dir}/CMakeLists.txt" content)
    string(FIND "${content}" "register_component" find_res)
    if(find_res EQUAL -1)
        set(${ret} 0 PARENT_SCOPE)
        return()
    endif()

    set(${ret} 1 PARENT_SCOPE)
endfunction()

function(find_components componet_dirs kconfigs configs found_main find_dir)
    set(_componet_dirs ${${componet_dirs}})
    set(_kconfigs ${${configs}})
    set(_configs ${${configs}})
    set(_found_main ${${found_main}})
    file(GLOB component_dirs ${find_dir})
    foreach(component_dir ${component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            get_filename_component(base_dir ${component_dir} NAME)
            message(STATUS "Component [${base_dir}] found: ${component_dir}")
            if(${base_dir} STREQUAL "main")
                set(_found_main 1)
            endif()
            list(APPEND _componet_dirs ${component_dir})
            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "          [${base_dir}] have Kconfig")
                list(APPEND _kconfigs ${component_dir}/Kconfig PARENT_SCOPE)
            endif()
            if(EXISTS ${component_dir}/config_defaults.mk)
                message(STATUS "          [${base_dir}] have defaults config")
                list(APPEND _configs --defaults "${component_dir}/config_defaults.mk" PARENT_SCOPE)
            endif()
        endif()
    endforeach()
    set(${componet_dirs} ${_componet_dirs} PARENT_SCOPE)
    set(${kconfigs} ${_kconfigs} PARENT_SCOPE)
    set(${configs} ${_configs} PARENT_SCOPE)
    set(${found_main} ${_found_main} PARENT_SCOPE)
endfunction()

function(get_python python version info_str)
    set(res 1)
    execute_process(COMMAND python3 --version RESULT_VARIABLE cmd_res OUTPUT_VARIABLE cmd_out)
    if(${cmd_res} EQUAL 0)
        set(${python} python3 PARENT_SCOPE)
        set(${version} 3 PARENT_SCOPE)
        set(${info_str} ${cmd_out} PARENT_SCOPE)
    else()
        execute_process(COMMAND python --version RESULT_VARIABLE cmd_res OUTPUT_VARIABLE cmd_out)
        if(${cmd_res} EQUAL 0)
            set(${python} python PARENT_SCOPE)
            set(${version} 2 PARENT_SCOPE)
            set(${info_str} ${cmd_out} PARENT_SCOPE)
        endif()
    endif()
endfunction(get_python python)


macro(project name)
    get_filename_component(current_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
    set(PROJECT_SOURCE_DIR ${current_dir})
    set(PROJECT_BINARY_DIR "${current_dir}/build")

    # Find components in SDK's components folder, register components
    find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${SDK_PATH}/components/*)
    find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${SDK_PATH}/components/3rd_party/*)
    find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${SDK_PATH}/components/ext_devs/*)
    # Find components in custom components folder, register components
    if(MAIXCDK_EXTRA_COMPONENTS_PATH)
        find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${MAIXCDK_EXTRA_COMPONENTS_PATH}/*)
    endif()
    if(PY_PKG_COMPONENTS_PATH)
        find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${PY_PKG_COMPONENTS_PATH}/*)
    endif()
    if(PY_USR_PKG_COMPONENTS_PATH)
        find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${PY_USR_PKG_COMPONENTS_PATH}/*)
    endif()
    # Find components in projects' shared components folder, register components
    find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${PROJECT_SOURCE_DIR}/../components/*)
    # Find components in project folder
    find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${PROJECT_SOURCE_DIR}/*)
    find_components(components_dirs components_kconfig_files kconfig_defaults_files_args found_main ${PROJECT_SOURCE_DIR}/components/*)

    if(NOT found_main)
        message(FATAL_ERROR "=================\nCan not find main component(folder) in project folder!!\n=================")
    endif()

    # # Find default config file
    # if(DEFAULT_CONFIG_FILE)
    #     message(STATUS "Project defaults config file:${DEFAULT_CONFIG_FILE}")
    #     list(APPEND kconfig_defaults_files_args --defaults "${DEFAULT_CONFIG_FILE}")
    #     if(EXISTS ${PROJECT_SOURCE_DIR}/.config.mk)
    #         message(STATUS "Find project defaults config(.config.mk)")
    #         list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/.config.mk")
    #     endif()
    # else()
    #     if(EXISTS ${PROJECT_SOURCE_DIR}/config_defaults.mk)
    #         message(STATUS "Find project defaults config(config_defaults.mk)")
    #         list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/config_defaults.mk")
    #     endif()
    #     if(EXISTS ${PROJECT_SOURCE_DIR}/.config.mk)
    #         message(STATUS "Find project defaults config(.config.mk)")
    #         list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/.config.mk")
    #     endif()
    # endif()

    # # Generate config file from Kconfig
    get_python(python python_version python_info_str)
    if(NOT python)
        message(FATAL_ERROR "python not found, please install python firstly(python3 recommend)!")
    endif()
    message(STATUS "python command: ${python}, version: ${python_info_str}")
    # string(REPLACE ";" " " components_kconfig_files "${kconfig_defaults_files_args}")
    # string(REPLACE ";" " " components_kconfig_files "${components_kconfig_files}")
    # message(FATAL_ERROR "----${kconfig_defaults_files_args}")
    # set(generate_config_cmd ${python} -u ${SDK_PATH}/tools/kconfig/genconfig.py
    #                         --kconfig "${SDK_PATH}/Kconfig"
    #                         ${kconfig_defaults_files_args}
    #                         --menuconfig False
    #                         --env "SDK_PATH=${SDK_PATH}"
    #                         --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
    #                         --env "MAIXCDK_EXTRA_COMPONENTS_PATH=${MAIXCDK_EXTRA_COMPONENTS_PATH}"
    #                         --env "PY_PKG_COMPONENTS_PATH=${PY_PKG_COMPONENTS_PATH}"
    #                         --env "PY_USR_PKG_COMPONENTS_PATH=${PY_USR_PKG_COMPONENTS_PATH}"
    #                         --env "BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    #                         --output makefile ${PROJECT_BINARY_DIR}/config/global_config.mk
    #                         --output cmake  ${PROJECT_BINARY_DIR}/config/global_config.cmake
    #                         --output header ${PROJECT_BINARY_DIR}/config/global_config.h
    #                         )
    # set(generate_config_cmd2 ${python} -u ${SDK_PATH}/tools/kconfig/genconfig.py
    #                         --kconfig "${SDK_PATH}/Kconfig"
    #                         ${kconfig_defaults_files_args}
    #                         --menuconfig True
    #                         --env "SDK_PATH=${SDK_PATH}"
    #                         --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
    #                         --env "MAIXCDK_EXTRA_COMPONENTS_PATH=${MAIXCDK_EXTRA_COMPONENTS_PATH}"
    #                         --env "PY_PKG_COMPONENTS_PATH=${PY_PKG_COMPONENTS_PATH}"
    #                         --env "PY_USR_PKG_COMPONENTS_PATH=${PY_USR_PKG_COMPONENTS_PATH}"
    #                         --env "BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    #                         --output makefile ${PROJECT_BINARY_DIR}/config/global_config.mk
    #                         --output cmake  ${PROJECT_BINARY_DIR}/config/global_config.cmake
    #                         --output header ${PROJECT_BINARY_DIR}/config/global_config.h
    #                         )
    # execute_process(COMMAND ${generate_config_cmd} RESULT_VARIABLE cmd_res)
    # if(NOT cmd_res EQUAL 0)
    #     message(FATAL_ERROR "Check Kconfig content")
    # endif()

    # Include configurations
    set(global_config_dir "${PROJECT_BINARY_DIR}/config")
    include(${global_config_dir}/global_config.cmake)
    if(WIN32)
        set(EXT ".exe")
        set(DL_EXT ".dll")
    else()
        set(EXT "")
        set(DL_EXT ".so")
    endif()

    include(${PROJECT_BINARY_DIR}/config/toolchain_config.cmake)

    # Config toolchain
    if(CONFIG_TOOLCHAIN_PATH MATCHES ".*.cmake$" AND NOT CMAKE_TOOLCHAIN_FILE)
        message("-- CONFIG_TOOLCHAIN_PATH is cmake file: ${CONFIG_TOOLCHAIN_PATH}")
        set(CMAKE_TOOLCHAIN_FILE ${CONFIG_TOOLCHAIN_PATH})
    endif()
    if(CMAKE_TOOLCHAIN_FILE)
        message("-- CMAKE_TOOLCHAIN_FILE set: ${CMAKE_TOOLCHAIN_FILE}")
    else()
        if(CONFIG_TOOLCHAIN_PATH OR CONFIG_TOOLCHAIN_PREFIX)
            if(CONFIG_TOOLCHAIN_PATH)
                if(WIN32)
                    file(TO_CMAKE_PATH ${CONFIG_TOOLCHAIN_PATH} CONFIG_TOOLCHAIN_PATH)
                endif()
                if(NOT IS_DIRECTORY ${CONFIG_TOOLCHAIN_PATH})
                    message(FATAL_ERROR "TOOLCHAIN_PATH set error:${CONFIG_TOOLCHAIN_PATH}")
                endif()
                set(path_split /)
                set(TOOLCHAIN_PATH ${CONFIG_TOOLCHAIN_PATH})
                message(STATUS "TOOLCHAIN_PATH:${CONFIG_TOOLCHAIN_PATH}")
                set(CMAKE_C_COMPILER "${CONFIG_TOOLCHAIN_PATH}${path_split}${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
                set(CMAKE_CXX_COMPILER "${CONFIG_TOOLCHAIN_PATH}${path_split}${CONFIG_TOOLCHAIN_PREFIX}g++${EXT}")
                set(CMAKE_ASM_COMPILER "${CONFIG_TOOLCHAIN_PATH}${path_split}${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
                set(CMAKE_LINKER "${CONFIG_TOOLCHAIN_PATH}${path_split}${CONFIG_TOOLCHAIN_PREFIX}ld${EXT}")
            else()
                message(STATUS "No TOOLCHAIN_PATH, only set TOOLCHAIN_PREFIX: ${CONFIG_TOOLCHAIN_PREFIX}")
                set(CMAKE_C_COMPILER "${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
                set(CMAKE_CXX_COMPILER "${CONFIG_TOOLCHAIN_PREFIX}g++${EXT}")
                set(CMAKE_ASM_COMPILER "${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
                set(CMAKE_LINKER "${CONFIG_TOOLCHAIN_PREFIX}ld${EXT}")
            endif()
        else()
            message(STATUS "use default toolchain: gcc")
            set(CMAKE_C_COMPILER "gcc${EXT}")
            set(CMAKE_CXX_COMPILER "g++${EXT}")
            set(CMAKE_ASM_COMPILER "gcc${EXT}")
            set(CMAKE_LINKER  "ld${EXT}")
        endif()
    endif()

    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)

    # # get glibc version write to ${CMAKE_BINARY_DIR}/config/glibc_version.txt
    # if(NOT IS_ABSOLUTE ${CMAKE_C_COMPILER})
    #     find_program(gcc_abs ${CMAKE_C_COMPILER})
    #     if(NOT gcc_abs)
    #         message(FATAL_ERROR "Can not find C compiler: ${CMAKE_C_COMPILER}")
    #     endif()
    # else()
    #     set(gcc_abs ${CMAKE_C_COMPILER})
    # endif()
    # set(get_glibc_version_cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/get_glibc_version.py ${gcc_abs} ${CMAKE_BINARY_DIR}/config/glibc_version.txt)
    # execute_process(${get_glibc_version_cmd} RESULT_VARIABLE cmd_res)
    # if(NOT cmd_res EQUAL 0)
    #     message(FATAL_ERROR "Get glibc version failed")
    # endif()


    # set(CMAKE_SYSTEM_NAME Generic) # set this flag may leads to dymamic(/shared) lib compile fail

    # Declare project # This function will clear flags!
    _project(${name} ASM C CXX)

    if(CMAKE_EXECUTABLE_SUFFIX STREQUAL ".js") # generate js ang html file for WASM
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
    endif()

    if(EXISTS "${PROJECT_PATH}/compile/compile_flags.cmake")
        include("${PROJECT_PATH}/compile/compile_flags.cmake")
    else()
        include("${SDK_PATH}/tools/cmake/compile_flags.cmake")
    endif()

    if(CONFIG_LIBS_LINK_STATIC)
        set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -static")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -static")
    endif()

    # add DEBUG or RELEASE flag globally
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG=1 -DRELEASE=0)
    else()
        add_definitions(-DRELEASE=1 -DDEBUG=0)
    endif()

    # clear components depends info
    set(clear_req_cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/components_depends.py clear ${CMAKE_BINARY_DIR}/config/components_depends.json)
    execute_process(${clear_req_cmd} RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Clear requirements info failed")
    endif()

    # Add dependence: update configfile, append time and git info for global config header file
    # we didn't generate build info for cmake and makefile for if we do, it will always rebuild cmake
    # everytime we execute make
    set(gen_build_info_config_cmd ${python}  ${SDK_PATH}/tools/kconfig/update_build_info.py
                                  --configfile header ${PROJECT_BINARY_DIR}/config/global_build_info_time.h ${PROJECT_BINARY_DIR}/config/global_build_info_version.h
                                  )
    add_custom_target(update_build_info COMMAND ${gen_build_info_config_cmd})

    # Sort component according to priority.conf config file
    set(component_priority_conf_file "${PROJECT_PATH}/compile/priority.conf")
    set(sort_components ${python}  ${SDK_PATH}/tools/cmake/sort_components.py
                                   ${component_priority_conf_file} ${components_dirs}
                        )
    execute_process(COMMAND ${sort_components} OUTPUT_VARIABLE component_dirs_sorted RESULT_VARIABLE cmd_res)
    if(cmd_res EQUAL 2)
        message(STATUS "No components priority config file")
        set(component_dirs_sorted ${components_dirs})
    elseif(cmd_res EQUAL 0)
        message(STATUS "Config components priority success")
    else()
        message(STATUS "Components priority config fail ${component_dirs_sorted}, check config file:${component_priority_conf_file}")
    endif()

    # guess components used
    set(cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/build.py get_valid_components ${SDK_PATH}/components ${SDK_PATH}/components/3rd_party ${SDK_PATH}/components/ext_devs ${MAIXCDK_EXTRA_COMPONENTS_PATH} ${PY_PKG_COMPONENTS_PATH} ${PY_USR_PKG_COMPONENTS_PATH} ${PROJECT_SOURCE_DIR}/../components ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/components)
    execute_process(${cmd} RESULT_VARIABLE cmd_res OUTPUT_VARIABLE component_valid)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Get valid components failed")
    endif()
    message("-- Only call enabled components: ${component_valid}")

    # Call CMakeLists.txt
    foreach(component_dir ${component_dirs_sorted})
        get_filename_component(base_dir ${component_dir} NAME)
        list(FIND component_valid ${base_dir} INDEX)
        if(INDEX EQUAL -1)
            message(STATUS "Ignore component ${base_dir}")
          else()
            add_subdirectory(${component_dir} ${base_dir} EXCLUDE_FROM_ALL)
            if(TARGET ${base_dir})
                add_dependencies(${base_dir} update_build_info) # add build info dependence
            else()
                message(STATUS "component ${base_dir} not enabled")
            endif()
          endif()
    endforeach()

    # update all global vars according to g_enabled_components

    # get target main's dependencies
    set(get_main_depends_cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/components_depends.py get ${CMAKE_BINARY_DIR}/config/components_depends.json main)
    execute_process(${get_main_depends_cmd} RESULT_VARIABLE cmd_res OUTPUT_VARIABLE main_depends)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "get main components requirements info from ${CMAKE_BINARY_DIR}/config/components_depends.json failed: ${main_depends}")
    endif()
    # remove \n
    list(APPEND main_depends main)
    set(g_enabled_components ${main_depends} CACHE INTERNAL "g_enabled_components")

    # a function to update global var according to enabled components
    function(update_global_var var_name)
        set(new_items)
        foreach(path ${${var_name}})
            string(REPLACE "+" ";" path ${path})
            list(GET path 0 component_name)
            list(GET path 1 item)
            list(FIND g_enabled_components ${component_name} enabled)
            if(enabled GREATER -1)
                list(APPEND new_items ${item})
            endif()
        endforeach()
        set(${var_name} ${new_items} CACHE INTERNAL ${var_name})
    endfunction()

    # update_global_var(g_files_download_info)
    update_global_var(g_includes)
    update_global_var(g_srcs)
    update_global_var(g_dynamic_libs)
    update_global_var(g_dist_ignore_libs)
    update_global_var(g_link_search_path)
    message(STATUS "libs not include in dist dir: ${g_dist_ignore_libs}")
    foreach(lib ${g_dist_ignore_libs})
        list(REMOVE_ITEM g_dynamic_libs ${lib})
    endforeach()

    # Download files according to g_files_download_info
    # if(g_files_download_info)
    #     message(STATUS "Start download files")
    #     set(download_cmd COMMAND ${python} -u ${SDK_PATH}/tools/file_downloader.py ${g_files_download_info})
    #     execute_process(${download_cmd} RESULT_VARIABLE cmd_res)
    #     if(NOT cmd_res EQUAL 0)
    #         message(FATAL_ERROR "Download files failed")
    #     endif()
    #     message(STATUS "Download files complete")
    # endif()

    # write global variables to ${CMAKE_BINARY_DIR}/config/cmake_global_variables.json
    set(write_global_vars_cmd COMMAND ${python} -u ${SDK_PATH}/tools/cmake/save_global_vars.py
                              -o ${CMAKE_BINARY_DIR}/config/cmake_global_vars.json
                              --var includes ${g_includes}
                              --var srcs ${g_srcs}
                              --var dynamic_libs ${g_dynamic_libs}
                              --var link_search_path ${g_link_search_path}
                            #   --var files_download_info ${g_files_download_info}
    )
    execute_process(${write_global_vars_cmd} RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Write global variables failed")
    endif()

    # Add lib search path to link flags
    foreach(abs_dir ${g_link_search_path})
        set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -L${abs_dir} -Wl,-rpath,${abs_dir}")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -L${abs_dir} -Wl,-rpath,${abs_dir}")
    endforeach()

    # Add menuconfig target for makefile
    # add_custom_target(menuconfig COMMAND ${generate_config_cmd2})

    # Create dummy source file exe_src.c to satisfy cmake's `add_executable` interface!
    set(exe_src ${CMAKE_BINARY_DIR}/exe_src.c)
    add_executable(${name} "${exe_src}")
    add_custom_command(OUTPUT ${exe_src} COMMAND ${CMAKE_COMMAND} -E touch ${exe_src} VERBATIM)
    add_custom_target(gen_exe_src DEPENDS "${exe_src}")
    add_dependencies(${name} gen_exe_src)

    # Add main component(lib)
    target_link_libraries(${name} main)

    # add doc generate
    set(doc_path ${SDK_PATH}/docs/api)
    set(cmake_global_vars_json ${CMAKE_BINARY_DIR}/config/cmake_global_vars.json)
    set(gen_doc_cmd ${python} -u ${SDK_PATH}/tools/doc_tool/gen_api.py --sdk_path ${SDK_PATH} --doc ${doc_path})
    execute_process(COMMAND ${gen_doc_cmd} RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Generate documentation failed")
    endif()

    # Add binary
    if(EXISTS "${PROJECT_PATH}/compile/gen_binary.cmake")
        include("${PROJECT_PATH}/compile/gen_binary.cmake")
    else()
        include("${SDK_PATH}/tools/cmake/gen_binary.cmake")
    endif()

endmacro()




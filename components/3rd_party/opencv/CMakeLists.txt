
# change version you should search all this document and change all version related string
if (PLATFORM_MAIXCAM)
    set(version_str "4.9.0")
    set(so_suffix_number "409")
elseif(PLATFORM_MAIXCAM2)
    set(version_str "4.11.0")
    set(so_suffix_number "411")
else()
    set(version_str "4.9.0")
    set(so_suffix_number "409")
endif()

# skip compile for local compile
if((NOT CONFIG_OPENCV_COMPILE_FROM_SOURCE) AND PLATFORM_LINUX AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
    # find local sdl
    if(CONFIG_OPENCV_DIR)
        set(OPENCV_DIR ${CONFIG_OPENCV_DIR})
    endif()
    find_package(OpenCV)
    list(APPEND ADD_INCLUDE ${OpenCV_INCLUDE_DIRS})
    list(APPEND ADD_REQUIREMENTS ${OpenCV_LIBRARIES})
    # remove opencv_freetype from OpenCV_LIBRARIES
    list(REMOVE_ITEM ADD_REQUIREMENTS opencv_freetype)
    if(NOT OpenCV_INCLUDE_DIRS)
        message(FATAL_ERROR "can not find OpenCV locally, will download source code to compile, you can install it by 'sudo apt install libopencv-dev libopencv-contrib-dev', or select OPENCV_COMPILE_FROM_SOURCE option to compile it from source code")
    endif()
endif()
if((PLATFORM_MAIXCAM OR PLATFORM_MAIXCAM2) AND NOT CONFIG_OPENCV_COMPILE_FROM_SOURCE AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/opencv-mobile")
        list(APPEND ADD_INCLUDE "opencv-mobile/include/opencv4")
        list(APPEND ADD_STATIC_LIB
                                "opencv-mobile/lib/libopencv_highgui.a"
                                "opencv-mobile/lib/libopencv_photo.a"
                                "opencv-mobile/lib/libopencv_video.a"
                                "opencv-mobile/lib/libopencv_features2d.a"
                                "opencv-mobile/lib/libopencv_imgproc.a"
                                "opencv-mobile/lib/libopencv_core.a"
        )
        list(APPEND ADD_REQUIREMENTS pthread dl)
        list(APPEND ADD_DEFINITIONS -fopenmp)
    else()
        if (PLATFORM_MAIXCAM)
            if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/opencv4_lib_maixcam")
                set(opencv_lib_dir "${CMAKE_CURRENT_LIST_DIR}/opencv4_lib_maixcam")
            elseif(CONFIG_TOOLCHAIN_PATH MATCHES "musl")
                set(opencv_lib_dir "${DL_EXTRACTED_PATH}/opencv/opencv4/opencv4_lib_maixcam_musl_${version_str}")
            endif()
        elseif(PLATFORM_MAIXCAM2)
            set(opencv_lib_dir "${DL_EXTRACTED_PATH}/opencv/opencv4/opencv4_lib_maixcam2_glibc_${version_str}")
        endif()
        if(opencv_lib_dir)
            list(APPEND ADD_INCLUDE "${opencv_lib_dir}/include/opencv4")
            if(CONFIG_LIBS_LINK_STATIC)
                set(opencv_libs
                                        "${opencv_lib_dir}/static_lib/libopencv_world.a"
                                        # "${opencv_lib_dir}/static_lib/opencv4/3rdparty/libade.a"
                                        # "${opencv_lib_dir}/static_lib/opencv4/3rdparty/libIlmImf.a"
                                        # "${opencv_lib_dir}/static_lib/opencv4/3rdparty/libittnotify.a"
                                        "${opencv_lib_dir}/static_lib/opencv4/3rdparty/liblibjpeg-turbo.a"
                                        "${opencv_lib_dir}/static_lib/opencv4/3rdparty/liblibopenjp2.a"
                                        "${opencv_lib_dir}/static_lib/opencv4/3rdparty/liblibpng.a"
                                        # "${opencv_lib_dir}/static_lib/opencv4/3rdparty/liblibtiff.a"
                                        # "${opencv_lib_dir}/static_lib/opencv4/3rdparty/liblibwebp.a"
                                        # "${opencv_lib_dir}/static_lib/opencv4/3rdparty/libquirc.a"
                                        "${opencv_lib_dir}/static_lib/opencv4/3rdparty/libzlib.a"
                )
                list(APPEND ADD_STATIC_LIB ${opencv_libs})
                list(APPEND ADD_REQUIREMENTS pthread dl)
                list(APPEND ADD_DEFINITIONS -fopenmp)
            else()
                set(opencv_libs "${opencv_lib_dir}/dl_lib/libopencv_core.so.${so_suffix_number}"
                                            # "${opencv_lib_dir}/dl_lib/libopencv_gapi.so.${so_suffix_number}"
                                            "${opencv_lib_dir}/dl_lib/libopencv_highgui.so.${so_suffix_number}"
                                            "${opencv_lib_dir}/dl_lib/libopencv_imgcodecs.so.${so_suffix_number}"
                                            "${opencv_lib_dir}/dl_lib/libopencv_imgproc.so.${so_suffix_number}"
                                            "${opencv_lib_dir}/dl_lib/libopencv_video.so.${so_suffix_number}"
                                            # "${opencv_lib_dir}/dl_lib/libopencv_freetype.so.${so_suffix_number}"
                )
                list(APPEND ADD_DYNAMIC_LIB ${opencv_libs})
                list(APPEND ADD_DIST_LIB_IGNORE ${opencv_libs})
                list(APPEND ADD_REQUIREMENTS pthread dl)
                list(APPEND ADD_DEFINITIONS -fopenmp)
            endif()
            foreach(lib ${opencv_libs})
                set_property(SOURCE ${lib} PROPERTY GENERATED 1)
            endforeach()
        endif()
    endif()
endif()


if(NOT ADD_INCLUDE)

    set(opencv_src_dir "${DL_EXTRACTED_PATH}/opencv/opencv4/opencv-${version_str}")
    set(opencv_cache_dir "${DL_PKGS_PATH}/opencv/cache")

    # list(APPEND ADD_REQUIREMENTS pthread dl atomic)
    list(APPEND ADD_DEFINITIONS -fopenmp)

    set(opencv_build_dir ${CMAKE_BINARY_DIR}/opencv4_build)
    set(opencv_install_dir ${CMAKE_BINARY_DIR}/opencv4_install)
    set(opencv_include_dir "${opencv_install_dir}/include/opencv4")
    set(opencv_header ${opencv_include_dir}/opencv2/opencv.hpp)

    # compile and include opencv lib
    if(PLATFORM_M2DOCK)
        set(opencv_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_m2dock.cmake)
    elseif(PLATFORM_LINUX)
        set(opencv_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux.cmake)
    elseif(PLATFORM_MAIXCAM OR PLATFORM_MAIXCAM2)
        set(opencv_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux_cross.cmake)
    else()
        message(FATAL_ERROR "No opencv toolchain config for this board, please edit to add opencv support for this board")
    endif()

    if(CONFIG_LIBS_LINK_STATIC)
        set(opencv_libs 
                    # "${opencv_install_dir}/lib/opencv4/3rdparty/libade.a"
                    # "${opencv_install_dir}/lib/opencv4/3rdparty/libIlmImf.a"
                    # "${opencv_install_dir}/lib/opencv4/3rdparty/libittnotify.a"
                    "${opencv_install_dir}/lib/opencv4/3rdparty/liblibjpeg-turbo.a"
                    "${opencv_install_dir}/lib/opencv4/3rdparty/liblibopenjp2.a"
                    "${opencv_install_dir}/lib/opencv4/3rdparty/liblibpng.a"
                    # "${opencv_install_dir}/lib/opencv4/3rdparty/liblibtiff.a"
                    # "${opencv_install_dir}/lib/opencv4/3rdparty/liblibwebp.a"
                    # "${opencv_install_dir}/lib/opencv4/3rdparty/libquirc.a"
                    "${opencv_install_dir}/lib/opencv4/3rdparty/libzlib.a"
        )
        set(opencv_world_lib "${opencv_install_dir}/lib/libopencv_world.a")
        list(APPEND ADD_STATIC_LIB ${opencv_world_lib}
                                    ${opencv_libs}
        )
        set_property(SOURCE ${opencv_world_lib} PROPERTY GENERATED 1)
        list(APPEND ADD_FILE_DEPENDS ${opencv_world_lib})
    else()
        set(opencv_libs "${opencv_install_dir}/lib/libopencv_core.so.${so_suffix_number}"
                        # "${opencv_install_dir}/lib/libopencv_gapi.so.${so_suffix_number}"
                        "${opencv_install_dir}/lib/libopencv_highgui.so.${so_suffix_number}"
                        "${opencv_install_dir}/lib/libopencv_imgcodecs.so.${so_suffix_number}"
                        "${opencv_install_dir}/lib/libopencv_imgproc.so.${so_suffix_number}"
                        "${opencv_install_dir}/lib/libopencv_video.so.${so_suffix_number}"
                        # "${opencv_install_dir}/lib/libopencv_freetype.so.${so_suffix_number}"
        )
        list(APPEND ADD_DYNAMIC_LIB ${opencv_libs})
        if(PLATFORM_MAIXCAM)
            list(APPEND ADD_DIST_LIB_IGNORE ${opencv_libs})
        elseif(PLATFORM_MAIXCAM2)
            # list(APPEND ADD_DIST_LIB_IGNORE ${opencv_libs})
        endif()
    endif()
    foreach(lib ${opencv_libs})
        set_property(SOURCE ${lib} PROPERTY GENERATED 1)
    endforeach()

    # if(NOT CONFIG_TOOLCHAIN_PATH)
    #     message(FATAL_ERROR "CONFIG_TOOLCHAIN_PATH not set which opencv toolchain.cmake need, please set it to your toolchain path")
    # endif()
    # vars for opencv to finx extra module dependecies

    include(${CMAKE_CURRENT_LIST_DIR}/opencv_conf.cmake)
    # TODO: add CMAKE_C_FLAGS and CMAKE_CXX_FLAGS to cmake args, but seems can conduct slower running speed for t-head flags .......
    set(compile_cmd COMMAND mkdir -p ${opencv_build_dir} && cd ${opencv_build_dir} && OPENCV_SRC_DIR=${opencv_src_dir} MAIXCDK_BUILD_DIR=${CMAKE_BINARY_DIR} SDK_PATH=${SDK_PATH} TOOLCHAIN_PATH=${CONFIG_TOOLCHAIN_PATH} TOOLCHAIN_PREFIX=${CONFIG_TOOLCHAIN_PREFIX} cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DOPENCV_DOWNLOAD_PATH=${opencv_cache_dir} -DCMAKE_TOOLCHAIN_FILE=${opencv_toolchain_file} -DCMAKE_INSTALL_PREFIX=${opencv_install_dir} ${config_args} ${opencv_src_dir} && make -j`nproc` install)
    message(STATUS "opencv compile cmd: ${compile_cmd}")
    add_custom_command(OUTPUT ${opencv_header} ${opencv_world_lib}
                # COMMAND mkdir -p ${opencv_build_dir} && cd ${opencv_build_dir} && SDK_PATH=${SDK_PATH} TOOLCHAIN_PATH=${CONFIG_TOOLCHAIN_PATH} cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_TOOLCHAIN_FILE=${opencv_toolchain_file} -DCMAKE_INSTALL_PREFIX=${opencv_install_dir} ${CMAKE_CURRENT_LIST_DIR}/opencv4 && make -j`nproc` install
                ${compile_cmd}
                COMMENT "Building opencv4 ..."
            )

    list(APPEND ADD_INCLUDE ${opencv_include_dir})
    # mark as generated file to prevent cmake error
    set_property(SOURCE ${opencv_include_dir} PROPERTY GENERATED 1)
    # seems target_link_libraries can not recoginize generated file, so add this empty source file
    list(APPEND ADD_FILE_DEPENDS ${opencv_header})
    set_property(SOURCE ${opencv_header} PROPERTY GENERATED 1)
endif()

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()


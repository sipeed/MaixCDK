
set(config_args
    "-DBUILD_ZLIB=ON"
    "-DBUILD_TIFF=OFF"
    "-DBUILD_OPENJPEG=ON"
    "-DBUILD_JASPER=OFF"
    "-DBUILD_JPEG=ON"
    "-DBUILD_PNG=ON"
    "-DBUILD_OPENEXR=OFF"
    "-DBUILD_WEBP=OFF"
    "-DBUILD_TBB=OFF"
    "-DBUILD_IPP_IW=OFF"
    "-DBUILD_ITT=OFF"
    "-DWITH_AVFOUNDATION=OFF"
    "-DWITH_CAP_IOS=OFF"
    "-DWITH_CAROTENE=OFF"
    "-DWITH_CPUFEATURES=OFF"
    "-DWITH_EIGEN=OFF"
    "-DWITH_FFMPEG=OFF"
    "-DWITH_GSTREAMER=OFF"
    "-DWITH_GTK=OFF"
    "-DWITH_IPP=OFF"
    "-DWITH_HALIDE=OFF"
    "-DWITH_VULKAN=OFF"
    "-DWITH_INF_ENGINE=OFF"
    "-DWITH_NGRAPH=OFF"
    "-DWITH_JASPER=OFF"
    "-DWITH_OPENJPEG=ON"
    "-DWITH_JPEG=ON"
    "-DWITH_WEBP=OFF"
    "-DWITH_OPENEXR=OFF"
    "-DWITH_PNG=ON"
    "-DWITH_TIFF=OFF"
    "-DWITH_OPENVX=OFF"
    "-DWITH_GDCM=OFF"
    "-DWITH_TBB=OFF"
    "-DWITH_HPX=OFF"
    "-DWITH_OPENMP=ON"
    "-DWITH_PTHREADS_PF=OFF"
    "-DWITH_V4L=OFF"
    "-DWITH_CLP=OFF"
    "-DWITH_OPENCL=OFF"
    "-DWITH_OPENCL_SVM=OFF"
    "-DWITH_VA=OFF"
    "-DWITH_VA_INTEL=OFF"
    "-DWITH_ITT=OFF"
    "-DWITH_PROTOBUF=OFF"
    "-DWITH_IMGCODEC_HDR=OFF"
    "-DWITH_IMGCODEC_SUNRASTER=OFF"
    "-DWITH_IMGCODEC_PXM=OFF"
    "-DWITH_IMGCODEC_PFM=OFF"
    "-DWITH_QUIRC=OFF"
    "-DWITH_ANDROID_MEDIANDK=OFF"
    "-DWITH_TENGINE=OFF"
    "-DWITH_ONNX=OFF"
    "-DWITH_TIMVX=OFF"
    "-DWITH_OBSENSOR=OFF"
    "-DWITH_CANN=OFF"
    "-DWITH_FLATBUFFERS=OFF"
    "-DBUILD_opencv_apps=OFF"
    "-DBUILD_ANDROID_PROJECTS=OFF"
    "-DBUILD_ANDROID_EXAMPLES=OFF"
    "-DBUILD_DOCS=OFF"
    "-DBUILD_EXAMPLES=OFF"
    "-DBUILD_PACKAGE=OFF"
    "-DBUILD_PERF_TESTS=OFF"
    "-DBUILD_TESTS=OFF"
    "-DBUILD_WITH_STATIC_CRT=OFF"
    "-DBUILD_FAT_JAVA_LIB=OFF"
    "-DBUILD_ANDROID_SERVICE=OFF"
    "-DBUILD_JAVA=OFF"
    "-DBUILD_OBJC=OFF"
    "-DENABLE_PRECOMPILED_HEADERS=OFF"
    "-DENABLE_FAST_MATH=ON"
    "-DCV_TRACE=OFF"
    "-DBUILD_opencv_java=OFF"
    "-DBUILD_opencv_gapi=OFF"
    "-DBUILD_opencv_objc=OFF"
    "-DBUILD_opencv_js=OFF"
    "-DBUILD_opencv_ts=OFF"
    "-DBUILD_opencv_python2=OFF"
    "-DBUILD_opencv_python3=OFF"
    "-DBUILD_opencv_dnn=OFF"
    "-DBUILD_opencv_imgcodecs=ON"
    "-DBUILD_opencv_videoio=OFF"
    "-DBUILD_opencv_calib3d=OFF"
    "-DBUILD_opencv_features2d=OFF"
    "-DBUILD_opencv_flann=OFF"
    "-DBUILD_opencv_objdetect=OFF"
    "-DBUILD_opencv_stitching=OFF"
    "-DBUILD_opencv_ml=OFF"
)

if(CONFIG_LIBS_LINK_STATIC)
    list(APPEND config_args "-DBUILD_opencv_world=ON")
    list(APPEND config_args "-DBUILD_SHARED_LIBS=OFF")
else()
    list(APPEND config_args "-DBUILD_opencv_world=OFF")
    list(APPEND config_args "-DBUILD_SHARED_LIBS=ON")
endif()

# set(freetype_install_dir "${CMAKE_BINARY_DIR}/freetype_install")
# set(harfbuzz_install_dir "${CMAKE_BINARY_DIR}/harfbuzz_install")
# list(APPEND config_args -Dfreetype_DIR="${freetype_install_dir}/lib/cmake/freetype")
# list(APPEND config_args -Dharfbuzz_DIR="${harfbuzz_install_dir}/lib/cmake/harfbuzz")
# # message("-- freetype_DIR: ${freetype_DIR}")
# # message("-- harfbuzz_DIR: ${harfbuzz_DIR}")
# # find_package(freetype REQUIRED)
# # find_package(harfbuzz REQUIRED)

# # set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR}/freetype_install/lib/cmake/freetype )

# list(APPEND config_args -DFREETYPE_FOUND=1)
# list(APPEND config_args -DFREETYPE_LINK_LIBRARIES="${freetype_install_dir}/lib/libfreetyped.a")
# list(APPEND config_args -DFREETYPE_LINK_LIBRARIES_XXXXX="${freetype_install_dir}/lib/libfreetyped.a")
# list(APPEND config_args -DFREETYPE_LIBRARIES="${freetype_install_dir}/lib/libfreetyped.a")
# list(APPEND config_args -DFREETYPE_LIBRARY_DIRS="${freetype_install_dir}/lib")
# list(APPEND config_args -DFREETYPE_INCLUDE_DIRS="${freetype_install_dir}/include/freetype2")

# list(APPEND config_args -DHARFBUZZ_FOUND=1)
# list(APPEND config_args -DHARFBUZZ_LINK_LIBRARIES="${harfbuzz_install_dir}/lib/libharfbuzz.a")
# list(APPEND config_args -DHARFBUZZ_LINK_LIBRARIES_XXXXX="${freetype_install_dir}/lib/libfreetyped.a")
# list(APPEND config_args -DHARFBUZZ_LIBRARIES="${harfbuzz_install_dir}/lib/libharfbuzz.a")
# list(APPEND config_args -DHARFBUZZ_LIBRARY_DIRS="${harfbuzz_install_dir}/lib")
# list(APPEND config_args -DHARFBUZZ_INCLUDE_DIRS="${harfbuzz_install_dir}/include/harfbuzz")
# list(APPEND config_args -DCMAKE_FIND_ROOT_PATH="${harfbuzz_install_dir}")

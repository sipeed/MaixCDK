

set(version_str "0.2")
set(unzip_path "${DL_EXTRACTED_PATH}/rlottie")
set(src_path "${unzip_path}/rlottie-${version_str}")

################# Add include #################
list(APPEND ADD_INCLUDE "${src_path}/inc"
                        "conf"
                        "${src_path}/src/lottie"
                        "${src_path}/src/vector"
                        "${src_path}/src/vector/freetype"
                        "${src_path}/src/vector/stb"
    )
# if(PLATFORM_MAIXCAM)
# list(APPEND ADD_INCLUDE "${src_path}/src/vector/pixman")
# endif()
# list(APPEND ADD_PRIVATE_INCLUDE "include_private")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
FILE(GLOB_RECURSE EXTRA_SRC  "${src_path}/src/binding/*.c"
                            "${src_path}/src/binding/*.cpp"
                            "${src_path}/src/lottie/*.c"
                            "${src_path}/src/lottie/*.cpp"
                            "${src_path}/src/vector/*.c"
                            "${src_path}/src/vector/*.cpp"
)
list(APPEND ADD_SRCS  ${EXTRA_SRC})
if(CONFIG_RLOTTIE_ARM)
    list(APPEND ADD_SRCS  ${EXTRA_SRC})
    FILE(GLOB_RECURSE EXTRA_SRC "${src_path}/src/vector/pixman/*.S")
    list(APPEND ADD_SRCS  ${EXTRA_SRC})
endif()
# FILE(GLOB EXTRA_SRC  "src/*.c")
# aux_source_directory(src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
# set(ADD_ASM_SRCS "src/asm.S")
# list(APPEND ADD_SRCS ${ADD_ASM_SRCS})
# SET_PROPERTY(SOURCE ${ADD_ASM_SRCS} PROPERTY LANGUAGE C) # set .S  ASM file as C language
# SET_SOURCE_FILES_PROPERTIES(${ADD_ASM_SRCS} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D BBBBB")
###############################################

###### Add required/dependent components ######
# list(APPEND ADD_REQUIREMENTS pixman)
###############################################

###### Add link search path for requirements/libs ######
# list(APPEND ADD_LINK_SEARCH_PATH "${CONFIG_TOOLCHAIN_PATH}/lib")
# list(APPEND ADD_REQUIREMENTS m)  # add system libs, pthread or m(math) lib for example
###############################################

############ Add static libs ##################
# if(CONFIG_COMPONENT1_INCLUDE_STATIC_LIB)
#     list(APPEND ADD_STATIC_LIB "lib/libtest.a")
# endif()
###############################################

############ Add dynamic libs ##################
# list(APPEND ADD_DYNAMIC_LIB "lib/arch/v831/libmaix_nn.so"
#                             "lib/arch/v831/libmaix_cam.so"
# )
###############################################

#### Add compile option for this component ####
#### Just for this component, won't affect other 
#### modules, including component that depend 
#### on this component
# list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

#### Add compile option for this component
#### Add components that depend on this component
# list(APPEND ADD_DEFINITIONS -DAAAAA222=1
#                             -DAAAAA333=1)
###############################################

############ Add static libs ##################
#### Update parent's variables like CMAKE_C_LINK_FLAGS
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()


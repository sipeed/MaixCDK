
########## set C flags #########
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wall -fPIC)
# GCC 15 defaults to C23, which breaks legacy C code in 3rd party components
# (e.g. `void f();` declarations called with arguments), keep the previous default.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 15)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -std=gnu17)
endif()
################################


###### set CXX(cpp) flags ######
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall -fPIC -std=c++17)
################################

# set(LINK_FLAGS -Wl,-EL) # (default little endian)
set(CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS}
                        ${LINK_FLAGS}
                        -Wl,-rpath=$ORIGIN/dl_lib
                        )
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${CMAKE_C_LINK_FLAGS}
                        )
# set(CMAKE_EXE_LINKER_FLAGS  ${CMAKE_EXE_LINKER_FLAGS}
#                             ${LINK_FLAGS}
#                             )
# set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS}
#                               ${LINK_FLAGS}
#                               )
# set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS}
#                               ${LINK_FLAGS}
#                               )


# Convert list to string
string(REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE ";" " " LINK_FLAGS "${LINK_FLAGS}")
string(REPLACE ";" " " CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}")
string(REPLACE ";" " " CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}")
# string(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
# string(REPLACE ";" " " CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
# string(REPLACE ";" " " CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")





list(APPEND ADD_INCLUDE "opencv_contrib/modules/freetype/include")
append_srcs_dir(ADD_SRCS "opencv_contrib/modules/freetype/src")
list(APPEND ADD_REQUIREMENTS opencv freetype harfbuzz)

register_component()


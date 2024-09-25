set(lib_include_dir "Clipper2Lib/include")
list(APPEND ADD_INCLUDE ${lib_include_dir})
aux_source_directory("Clipper2Lib/src" ADD_SRCS)

register_component()

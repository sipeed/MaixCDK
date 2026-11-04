############### Add include ###################
list(APPEND ADD_INCLUDE "include"
    )
list(APPEND ADD_PRIVATE_INCLUDE "")
###############################################

############ Add source files #################
append_srcs_dir(ADD_SRCS "src")       # append source file in src dir to var ADD_SRCS
###############################################

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS basic nn vision)
###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

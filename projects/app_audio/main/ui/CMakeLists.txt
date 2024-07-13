SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    ui.c
    imgbtn_ui/*.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.cpp)

add_library(ui ${SOURCES})

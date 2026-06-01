SET(SOURCES ui_Screen1.c
    ui_Screen2.c
    ui_Screen3.c
    ui_Screen4.c
    ui_Screen5.c
    ui_Screen6.c
    ui_Screen7.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    ui_img_calibration_png.c
    ui_img_screen2_png.c)

add_library(ui ${SOURCES})

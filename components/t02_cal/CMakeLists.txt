idf_component_register(SRCS "t02_cal.c" "t02_cal_render.c" INCLUDE_DIRS "include" REQUIRES t02_touch_map)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)

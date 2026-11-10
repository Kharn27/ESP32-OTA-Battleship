idf_component_register(
    SRCS "warship_panel_profile.c"
    INCLUDE_DIRS "include"
)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)

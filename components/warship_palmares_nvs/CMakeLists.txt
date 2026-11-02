idf_component_register(SRCS "warship_palmares_nvs.c"
                       INCLUDE_DIRS "include"
                       REQUIRES warship_palmares nvs_flash)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

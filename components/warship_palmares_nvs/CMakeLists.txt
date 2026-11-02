idf_component_register(SRCS "warship_palmares_nvs.c"
                       INCLUDE_DIRS "include"
                       REQUIRES warship_palmares nvs_flash)
# ESP-IDF's newlib wrapper stdio.h uses the GNU #include_next extension.
# Only the IDF adapter disables -pedantic; the pure C99 core and host fake
# adapter remain strict -Wall -Wextra -Werror -pedantic in CI.
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -Wno-pedantic)

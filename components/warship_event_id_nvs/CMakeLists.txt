idf_component_register(SRCS "warship_event_id_nvs.c" INCLUDE_DIRS "include"
                       REQUIRES warship_event_bridge nvs_flash)
# ESP-IDF's newlib wrapper stdio.h uses GNU #include_next.
# Match D08 NVS adapter only here; C99 bridge and host-stub tests remain
# strict -Wall -Wextra -Werror -pedantic.
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -Wno-pedantic)

idf_component_register(SRCS "warship_event_id_nvs.c" INCLUDE_DIRS "include"
                       REQUIRES warship_event_bridge nvs_flash)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

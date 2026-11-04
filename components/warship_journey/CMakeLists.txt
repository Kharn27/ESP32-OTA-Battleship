idf_component_register(SRCS "warship_journey.c"
                       INCLUDE_DIRS "include"
                       REQUIRES warship_combat_ui warship_event_bridge)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

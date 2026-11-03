idf_component_register(SRCS "warship_event_bridge.c"
                       INCLUDE_DIRS "include" REQUIRES warship_palmares)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

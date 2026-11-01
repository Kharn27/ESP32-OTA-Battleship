idf_component_register(SRCS "warship_lifecycle.c"
                       INCLUDE_DIRS "include"
                       REQUIRES warship_solo warship_fleet)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

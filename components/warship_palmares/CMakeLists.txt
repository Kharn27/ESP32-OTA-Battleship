idf_component_register(SRCS "warship_palmares.c"
                       INCLUDE_DIRS "include"
                       REQUIRES warship_lifecycle)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

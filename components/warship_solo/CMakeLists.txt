idf_component_register(SRCS "warship_solo.c" INCLUDE_DIRS "include"
                       REQUIRES warship_battle warship_captain)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

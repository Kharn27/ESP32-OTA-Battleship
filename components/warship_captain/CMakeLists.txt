idf_component_register(SRCS "warship_captain.c" INCLUDE_DIRS "include" REQUIRES warship_battle)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -pedantic)

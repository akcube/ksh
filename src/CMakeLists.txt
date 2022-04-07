include_directories(${KSH_SOURCE_DIR}/include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${KSH_BINARY_DIR}/bin/)
add_executable(ksh shell.c builtins.c colors.c error_handlers.c execute.c history.c ls.c parsing.c proclist.c prompt.c signal_handlers.c utils.c vector.c)
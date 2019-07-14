# -----------------------------------------------------------------------------
# Building test projects.
# -----------------------------------------------------------------------------
add_executable("test_hook" "${CMAKE_CURRENT_LIST_DIR}/test_hook.cpp")
target_link_libraries("test_hook" PRIVATE "htp")
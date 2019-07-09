# -----------------------------------------------------------------------------
# Building test projects.
# -----------------------------------------------------------------------------
add_executable("test_server" "${CMAKE_CURRENT_LIST_DIR}/test_server.cpp")
add_executable("test_hook" "${CMAKE_CURRENT_LIST_DIR}/test_hook.cpp")
target_link_libraries("test_server" PRIVATE "htp")
target_include_directories("test_server" PRIVATE "${PROJECT_SOURCE_DIR}/utils")
target_link_libraries("test_hook" PRIVATE "htp")
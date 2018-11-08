
enable_language(ASM_MASM)

if(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
    set(TRAMPOLINE_SOURCES ${AGENT_SOURCES} "${CMAKE_CURRENT_LIST_DIR}/trampoline_x64.asm")
elseif( CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    set(TRAMPOLINE_SOURCES ${AGENT_SOURCES} "${CMAKE_CURRENT_LIST_DIR}/trampoline_x32.asm")
    set_source_files_properties(
    "${CMAKE_CURRENT_LIST_DIR}/trampoline_x32.asm"
    PROPERTIES
    COMPILE_FLAGS "/safeseh"
)
else()
    message(FATAL_ERROR "Unsupported target platform. Please generate your project with -A x64 or -A Win32")
endif()

target_sources("htp" PRIVATE "${CMAKE_CURRENT_LIST_DIR}/hooking.cpp"
                             "${CMAKE_CURRENT_LIST_DIR}/htp.cpp"
                             "${CMAKE_CURRENT_LIST_DIR}/trampoline.cpp"
                             ${TRAMPOLINE_SOURCES})
target_link_libraries("htp" PUBLIC "Zydis")
target_link_libraries("htp" PUBLIC "Zycore")
cmake_minimum_required (VERSION 3.21)

project (wimer CXX)

add_subdirectory(lib)

IF (WIN32)
    enable_language("RC")
    set (_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/wimer.rc)
ENDIF (WIN32)

add_executable(wimer main.cpp ${_RESOURCES})
target_link_libraries(wimer libwimer)

# add_custom_command(TARGET wimer POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:wimer> $<TARGET_FILE_DIR:wimer>
#   COMMAND_EXPAND_LISTS
# )

IF (WIN32)
    set_target_properties(wimer PROPERTIES COMPILE_DEFINITIONS_DEBUG "_WINDOWS")
    set_target_properties(wimer PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(wimer PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_WINDOWS")
    set_target_properties(wimer PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDWOS")
    set_target_properties(wimer PROPERTIES COMPILE_DEFINITIONS_RELEASE "_WINDOWS")
    set_target_properties(wimer PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(wimer PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "_WINDOWS")
    set_target_properties(wimer PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
ENDIF (WIN32)

set(INCROOT ${PROJECT_SOURCE_DIR}/example)

set(SRC 
    ${INCROOT}/main.cpp
)

add_executable(main ${SRC})
target_link_libraries(main PUBLIC ecl)

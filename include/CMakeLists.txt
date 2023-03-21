set(INCROOT ${PROJECT_SOURCE_DIR}/include/TMBEL/)

set(SRC
# Global file
    ${PROJECT_SOURCE_DIR}/include/TMBEL.hpp

# Files

)

add_library(tmbel ${SRC})

set_target_properties(tmbel PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(tmbel PUBLIC ${PROJECT_SOURCE_DIR}/include/)

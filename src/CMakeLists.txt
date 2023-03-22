set(INCROOT ${PROJECT_SOURCE_DIR}/include/TMBEL/)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/)

set(SRC
# Global file
    ${PROJECT_SOURCE_DIR}/include/TMBEL.hpp

# Files
    ${INCROOT}/handler.hpp
    ${SRCROOT}/handler.cpp
    ${INCROOT}/utils.hpp
    ${SRCROOT}/utils.cpp
)

add_library(tmbel ${SRC})

target_include_directories(tmbel PUBLIC ${PROJECT_SOURCE_DIR}/include/)

set_target_properties(tmbel PROPERTIES LINKER_LANGUAGE CXX)


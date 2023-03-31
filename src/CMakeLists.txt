set(INCROOT ${PROJECT_SOURCE_DIR}/include/TMBEL/)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/)

set(SRC
# Global file
    ${PROJECT_SOURCE_DIR}/include/TMBEL.hpp

# Files
    ${INCROOT}/singleton.hpp
    ${SRCROOT}/singleton.cpp
    ${INCROOT}/multithread_list.hpp
    ${SRCROOT}/multithread_list.cpp
    ${INCROOT}/lock_handler.hpp
    ${SRCROOT}/lock_handler.cpp
    ${INCROOT}/handler.hpp
    ${SRCROOT}/handler.cpp
    ${INCROOT}/utils.hpp
    ${SRCROOT}/utils.cpp
    ${INCROOT}/event_queue.hpp
    ${INCROOT}/controller.hpp
    ${INCROOT}/global_container.hpp
)

add_library(tmbel ${SRC})

target_include_directories(tmbel PUBLIC ${PROJECT_SOURCE_DIR}/include/)

set_target_properties(tmbel PROPERTIES LINKER_LANGUAGE CXX)
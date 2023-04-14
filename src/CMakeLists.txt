set(INCROOT ${PROJECT_SOURCE_DIR}/include/ECL/)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/)

set(SRC
# Global file
    ${PROJECT_SOURCE_DIR}/include/ECL.hpp

# Files
    ${INCROOT}/impl/thread.hpp
    ${SRCROOT}/impl/thread.cpp
    ${INCROOT}/impl/singleton.hpp
    ${SRCROOT}/impl/singleton.cpp
    ${INCROOT}/impl/ts_list.hpp
    ${INCROOT}/impl/observer.hpp
    ${INCROOT}/impl/lock_handler.hpp
    ${SRCROOT}/impl/lock_handler.cpp
    ${INCROOT}/process_list.hpp
    ${SRCROOT}/process_list.cpp
    ${INCROOT}/handler.hpp
    ${SRCROOT}/handler.cpp
    ${INCROOT}/utils.hpp
    ${SRCROOT}/utils.cpp
    ${INCROOT}/event_queue.hpp
    ${INCROOT}/controller.hpp
    ${INCROOT}/event_bus_container.hpp
)

add_library(ecl ${SRC})

target_include_directories(ecl PUBLIC ${PROJECT_SOURCE_DIR}/include/)

set_target_properties(ecl PROPERTIES LINKER_LANGUAGE CXX)
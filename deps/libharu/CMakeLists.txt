message(STATUS "Fetching libharu...")

set(BUILD_SHARED_LIBS FALSE)

FetchContent_MakeAvailable(libharu)

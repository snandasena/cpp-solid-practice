
find_package(GTest)
include_directories(${GTEST_INCLUDE_DIRS})

set(SOURCE
        main.cpp)


add_executable(runTests ${SOURCE})
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
project(server)

find_package(Boost COMPONENTS system REQUIRED)

set(SOURCE
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/external)

add_executable(${PROJECT_NAME} ${SOURCE})


add_executable(bosotserver boost_server.cpp)
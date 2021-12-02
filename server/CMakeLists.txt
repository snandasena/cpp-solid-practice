project(server)


find_package(Boost COMPONENTS system REQUIRED)

set(SOURCE
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/ex)
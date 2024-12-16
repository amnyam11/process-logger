cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(COMMON_SOURCES
    common.cpp
    common.hpp
)

set(CHILD_SOURCES
    child.cpp
    my_shmem.hpp
)

set(TEST_SOURCES
    test.cpp
    my_shmem.hpp
)

# Собираем common как библиотеку
add_library(common ${COMMON_SOURCES})

# Собираем child и test как исполняемые файлы и связываем их с common
add_executable(child ${CHILD_SOURCES})
target_link_libraries(child PRIVATE common)

add_executable(test ${TEST_SOURCES})
target_link_libraries(test PRIVATE common)
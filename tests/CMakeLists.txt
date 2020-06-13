set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(ExternalProject)

ExternalProject_Add(
    gtest
    URL http://googletest.googlecode.com/files/gtest-1.7.0.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    URL_MD5 2d6ec8ccdf5c46b05ba54a9fd1d130d7
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(gtest source_dir binary_dir)

add_library(libgtest INTERFACE)
add_dependencies(libgtest gtest)
target_link_libraries(libgtest
    INTERFACE Threads::Threads
              "${binary_dir}/libgtest_main.a"
              "${binary_dir}/libgtest.a")
target_include_directories(libgtest INTERFACE "${source_dir}/include")

enable_testing()

foreach(case_file ${test_cases})
    get_filename_component( case_name ${case_file} NAME_WE )
    set (case_name test_${case_name})
    add_executable(${case_name} ${case_file})
    target_link_libraries(${case_name} libgtest MyLibrary)
    add_test(NAME ${case_name}
             COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${case_name}
             WORKING_DIRECTORY
             ${PROJECT_BINARY_DIR}
             )

endforeach()
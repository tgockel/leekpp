cmake_minimum_required(VERSION 3.0)
project(leekpp)

include_directories(src)

enable_testing()
file(GLOB_RECURSE test_cpps RELATIVE_PATH "." "src/leekpp_tests/*.cpp")
foreach(cpp ${test_cpps})
  get_filename_component(friendly_name ${cpp} NAME_WE)
  add_executable(${friendly_name} ${cpp})
  add_test(${friendly_name} ${friendly_name})
endforeach()

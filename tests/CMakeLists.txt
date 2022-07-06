cmake_minimum_required(VERSION 3.0.0)
project(mod-test VERSION 0.1.0)

include(CTest)
enable_testing()

set(CTEST_OUTPUT_ON_FAILURE ON)

add_compile_definitions(LOCAL_CPP_TEST)
add_executable(mod-test main_test.cpp ../ScanUtil.cpp ../SMSMessage.cpp)
add_test(NAME test1 COMMAND mod-test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
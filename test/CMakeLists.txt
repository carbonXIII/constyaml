include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)
function(add_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} gtest_main)
  target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/include)
  gtest_discover_tests(${name})
endfunction()

add_test(test_const_list)
add_test(test_const_dict)
add_test(test_yaml)

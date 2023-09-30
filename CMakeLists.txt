cmake_minimum_required(VERSION 3.14)
project(alpinef1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

link_directories("/opt/homebrew/lib")
include_directories("/opt/homebrew/include")

find_package(jsoncpp REQUIRED)
add_library(jsonFunctions jsonFunctions.cpp)
target_include_directories(jsonFunctions PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main main.cpp dataPoint.cpp dataChannel.cpp dataInput.cpp extractedSubChannel.cpp timer.cpp dataCollector.cpp)
target_link_libraries(main PRIVATE jsoncpp jsonFunctions)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(tests tests.cpp dataPoint.cpp dataInput.cpp dataChannel.cpp extractedSubChannel.cpp dataCollector.cpp timer.cpp)
target_link_libraries(tests gtest_main jsonFunctions jsoncpp)
add_test(NAME test_suite COMMAND tests)

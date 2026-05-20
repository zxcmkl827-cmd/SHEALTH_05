cmake_minimum_required(VERSION 3.14)
project(CoreProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main library
add_library(shealth_lib
    src/main/cpp/StandardBmiClassifier.cpp
    src/main/cpp/SHealth.cpp
)
target_include_directories(shealth_lib PUBLIC src/main/cpp)

# Main executable
add_executable(SHealthBMI src/main/cpp/SHealthBMI.cpp)
target_link_libraries(SHealthBMI PRIVATE shealth_lib)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_Declare(
    ApprovalTests
    GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
    GIT_TAG v.10.13.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(ApprovalTests)

enable_testing()

# Test executable
add_executable(SHealthBMITest src/test/cpp/SHealthBMITest.cpp)
target_link_libraries(SHealthBMITest PRIVATE shealth_lib GTest::gtest_main)

add_executable(SHealthGoldenMasterTest
    src/test/cpp/ApprovalTestsMain.cpp
    src/test/cpp/TexttestFixture.cpp
)
target_link_libraries(SHealthGoldenMasterTest PRIVATE
    shealth_lib
    ApprovalTests::ApprovalTests
    GTest::gtest
)
target_compile_definitions(SHealthGoldenMasterTest PRIVATE
    SHEALTH_GOLDEN_DATA_FILE="${CMAKE_SOURCE_DIR}/shealth.dat"
    SHEALTH_GOLDEN_EXECUTABLE_FILE="$<TARGET_FILE:SHealthBMI>"
    SHEALTH_GOLDEN_WORKING_DIR="${CMAKE_SOURCE_DIR}"
)

include(GoogleTest)
gtest_discover_tests(SHealthBMITest)
gtest_discover_tests(SHealthGoldenMasterTest)

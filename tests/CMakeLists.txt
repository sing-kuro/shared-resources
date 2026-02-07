include(FetchContent)

# Fetch GTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
# Prevent GTest from building its own tests
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Define test executable
add_executable(test_shared_resources shared_resources.cpp)
target_link_libraries(test_shared_resources PRIVATE shared_resources GTest::gtest_main)

# Register tests
include(GoogleTest)
gtest_discover_tests(test_shared_resources)

cmake_minimum_required(VERSION 3.20)
project(t9_result)


# オプションの設定
option(T9_RESULT_BUILD_TESTS "Build tests" OFF)


# t9_result
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

## C++17以上を必須とする
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

## UTF-8をデフォルトの文字セットとする
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

## 警告レベルを最大にする
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

## 例外を無効にする
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/EHa->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-exceptions>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/D_HAS_EXCEPTIONS=0>
)

## RTTIを無効にする
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/GR->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-rtti>
)


if(T9_RESULT_BUILD_TESTS)
    enable_testing()

    include(FetchContent)

    # Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.16.x
        GIT_SHALLOW TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # t9_result_test
    add_executable(${PROJECT_NAME}_test
        tests/result_test.cpp
    )
    target_link_libraries(${PROJECT_NAME}_test PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test)
endif()

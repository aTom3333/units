include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.9
)

FetchContent_MakeAvailable(Catch2)

add_executable(
    tests
    test_magnitude.cpp
    test_main.cpp
    test_prime.cpp
    test_quantity.cpp
    test_unit.cpp
    unit_definition.h
)

target_link_libraries(tests PUBLIC units)
target_link_libraries(tests PUBLIC Catch2::Catch2)

target_compile_options(
    tests
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Werror
            -Wall
            -Wextra
            -Wconversion
            -pedantic
            >
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /WX
            >
)

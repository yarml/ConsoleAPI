cmake_minimum_required(VERSION 3.12)

project(ConsoleAPI)

set(ConsoleAPI_Target "cn_api" PARENT_SCOPE)

file(
    GLOB_RECURSE
    CNAPI_SRC
    "src/*.c"
)
file(
    GLOB_RECURSE
    CNAPI_INC
    "inc/*.h" "src/*.h"
)

add_library(
    ${ConsoleAPI_Target}
    STATIC
    ${CN_API_SRC}
    ${CN_API_INC}
)

target_include_directories(${ConsoleAPI_Target} PUBLIC)

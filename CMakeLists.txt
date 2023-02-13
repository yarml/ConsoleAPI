cmake_minimum_required(VERSION 3.12)

project(ConsoleAPI)

set(internal_ConsoleAPI_Target "cn_api")
set(ConsoleAPI_Target ${internal_ConsoleAPI_Target} PARENT_SCOPE)

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
    ${internal_ConsoleAPI_Target}
    STATIC
    ${CNAPI_SRC}
    ${CNAPI_INC}
)

target_include_directories(${internal_ConsoleAPI_Target} PUBLIC "inc/")

set(APP_COMMON_NAME "${PROJECT_NAME}_app_common")

add_library(${APP_COMMON_NAME})
add_library(app::common ALIAS ${APP_COMMON_NAME})

target_compile_definitions(${APP_COMMON_NAME} PRIVATE CMAKE_EXPORT_COMPILE_COMMANDS=1)
target_include_directories(${APP_COMMON_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(input_parser)

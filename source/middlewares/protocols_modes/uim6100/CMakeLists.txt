# Protocol UIM6100 specific configuration
file(GLOB_RECURSE UIM6100_SOURCES *.c)
file(GLOB_RECURSE UIM6100_HEADERS *.h)

add_library(PROTOCOL_TARGET STATIC ${UIM6100_SOURCES})
target_include_directories(PROTOCOL_TARGET PRIVATE ${PROJECT_DIR}/middlewares/protocols_modes/uim6100)
target_compile_definitions(PROTOCOL_TARGET PRIVATE PROTOCOL_UIM_6100=1)

# # set(DEMO_MODE_SOURCES demo_mode.c demo_mode.h)
# # # set(PROTOCOL_NAME "UIM_6100")
# # add_library(DEMO_MODE_SOURCES STATIC ${DEMO_MODE_SOURCES})
# # target_compile_definitions(DEMO_MODE_SOURCES PUBLIC DEMO_MODE=1)
# # # Указываем пути к заголовочным файлам компонента
# # target_include_directories(DEMO_MODE_SOURCES PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# # add_library(demo_mode_lib STATIC demo_mode.c demo_mode.h)

# # target_include_directories(demo_mode_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# #target_compile_definitions(demo_mode_lib PUBLIC DEMO_MODE=1)

# # Указываем исходные файлы для demo_mode
# set(DEMO_MODE_SOURCES
#     demo_mode.c
# )

# # Создаем библиотеку или добавляем файлы в проект
# add_library(demo_mode_lib STATIC ${DEMO_MODE_SOURCES})

# # Настройка включения заголовочных файлов
# target_include_directories(demo_mode_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

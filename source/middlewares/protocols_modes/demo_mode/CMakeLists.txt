# # demo_mode/CMakeLists.txt

# # cmake_minimum_required(VERSION 3.22)

# project(demo_mode)

# set(MCU_FAMILY STM32F1xx)
# set(MCU_MODEL STM32F103xB)

# set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/demo_mode.c)
# set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/demo_mode.h)
# add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/CMSIS/Include
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Core/Inc
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/${MCU_FAMILY}_HAL_Driver/Inc
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
#     ${CMAKE_CURRENT_SOURCE_DIR})


# # add_library(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/demo_mode.c)
# # message(STATUS "Demo mode source file:" ${CMAKE_CURRENT_SOURCE_DIR}/demo_mode.c)
# # target_include_directories(${PROJECT_NAME} PUBLIC


# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../peripherals # Подключаем заголовки периферии
# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../peripherals/interfaces
# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../peripherals/menu
# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/CMSIS/Device/ST/STM32F1xx/Include
# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/CMSIS/Include
# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/STM32F1xx_HAL_Driver/Inc
# #     # ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Core/Inc

# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/CMSIS/Include
# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Core/Inc
# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/${MCU_FAMILY}_HAL_Driver/Inc
# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy

# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../peripherals # Подключаем заголовки периферии
# #     ${CMAKE_CURRENT_SOURCE_DIR}/../../../peripherals/interfaces

# #     ${CMAKE_CURRENT_SOURCE_DIR}
# # )



# target_compile_definitions(${PROJECT_NAME} PUBLIC
#     ${MCU_MODEL}
#     DEMO_MODE=1 USE_HAL_DRIVER)


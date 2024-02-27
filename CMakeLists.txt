
cmake_minimum_required(VERSION 3.13)

add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project LANGUAGES C CXX)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
#add_compile_options(-Werror -Wall -Wextra -Wnull-dereference)
#add_compile_options(-Wall)
add_executable(oXs_receiver)

#pico_generate_pio_header(oXs_receiver ${CMAKE_CURRENT_LIST_DIR}/src/uart_crsf_rx.pio )


file(GLOB oXs_receiver_SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

target_sources(oXs_receiver PRIVATE ${oXs_receiver_SRC}  )

target_link_libraries(oXs_receiver PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_i2c
        hardware_watchdog
        hardware_timer
        hardware_dma
        hardware_irq
        hardware_adc
        hardware_pwm
        hardware_spi
        )

#enable printf on usb
pico_enable_stdio_usb(oXs_receiver 1)
pico_enable_stdio_uart(oXs_receiver 0)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(oXs_receiver)


add_custom_command(TARGET oXs_receiver POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy oXs_receiver.uf2 ${CMAKE_SOURCE_DIR}   
)
#  ARGS -E copy oXs.uf2 ${CMAKE_SOURCE_DIR} it was with -E   
# copy the uf2 file to E: drive
add_custom_command(TARGET oXs_receiver POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy oXs_receiver.uf2 E: )

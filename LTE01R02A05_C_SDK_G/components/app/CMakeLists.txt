# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#

add_subdirectory_if_exist(init)

add_subdirectory_if_exist(peripheral)
add_subdirectory_if_exist(sdmmc)

add_subdirectory_if_exist(t2n)
add_subdirectory_if_exist(rtk)
add_subdirectory_if_exist(modbus_rtu_slave)
# add_subdirectory_if_exist(modbus_master)
add_subdirectory_if_exist(uart1_rs485)
add_subdirectory_if_exist(rs485)

add_subdirectory_if_exist(usb_acm_netif)
# add_subdirectory_if_exist(esp-serial-flasher)

# add_subdirectory_if_exist(lvgl_lib)
# add_subdirectory_if_exist(lvgl_app)
add_subdirectory_if_exist(ec600u_bl0939)
add_subdirectory_if_exist(esp32_flash)
add_subdirectory_if_exist(http_fota)
add_subdirectory_if_exist(lis3dsh)
# add_subdirectory_if_exist(lora)
# add_subdirectory_if_exist(lora_test)
# add_subdirectory_if_exist(GT911)
# add_subdirectory_if_exist(voice)
add_subdirectory_if_exist(audio)
add_subdirectory_if_exist(gnss)
add_subdirectory_if_exist(esp32_network)
add_subdirectory_if_exist(mqtt_test)
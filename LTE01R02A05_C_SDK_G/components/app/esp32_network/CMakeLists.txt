

set(target esp32_network)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PUBLIC common  driver/serial driver/transport driver/transport/spi driver/netif driver/network 
app_host app_host/control app_host/data 
common_based/include common_based/protobuf-c 
port/include 
component/include
control_lib/include 
control_lib/src/include)
include_directories(${SOURCE_TOP_DIR}/components/app/t2n/include)

# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/common)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/control_lib/include)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/control_lib/scr/include)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/common_based/include)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/common_based/protobuf-c)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/driver/serial)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/driver/transport)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/driver/transport/sdio)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/driver/transport/spi)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/driver/netif)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/driver/network)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/app_host)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/app_host/control)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/app_host/data)

# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/port/include)
# include_directories(${SOURCE_TOP_DIR}/components/app/esp32_network/component/include)

# include_directories(${SOURCE_TOP_DIR}/components/kernel/freertos/include)
# include_directories(${SOURCE_TOP_DIR}/components/kernel/chip/8550)

target_link_libraries(${target} PUBLIC )
target_link_libraries(${target} PRIVATE  rtk)
target_link_libraries(${target} PRIVATE  t2n)


target_sources(${target} PUBLIC
				app_host/control/control_utils.c
				app_host/control/control.c
				# app_host/data/control_api.c
				app_host/data/arp_server_stub.c
				app_host/app_main_api.c
				app_host/app_main.c
				common/common.c
				common/stats.c
				common/util.c
				component/src/esp_queue.c
				control_lib/src/ctrl_api.c
				control_lib/src/ctrl_core.c
				driver/network/netdev_api.c
				driver/network/netdev_stub.c
				driver/serial/serial_ll_if.c
				driver/serial/serial_if.c
				driver/transport/transport_drv.c
				# # driver/transport/sdio/sdio_api.c
				# # driver/transport/sdio/sdio_drv.c
				# # driver/transport/sdio/sdio_host.c
				# # driver/transport/sdio/sdio_ll.c
				driver/transport/spi/spi_drv.c
				port/src/platform_wrapper.c
				common_based/esp_hosted_config.pb-c.c
				common_based/protobuf-c/protobuf-c.c

)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
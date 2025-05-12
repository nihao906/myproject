set(target esp32_flash)    

add_library(${target} STATIC) 
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir}) 
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC) 
target_include_directories(${target} PUBLIC inc bootloader/inc bootloader/binaries)

target_link_libraries(${target} PRIVATE  sdmmc)
target_link_libraries(${target} PRIVATE  t2n modbus_rtu_slave)

target_sources(${target} 
                PUBLIC     
                esp32_flash.c 
                bootloader/ec600u_port.c
                bootloader/esp_loader.c 
                bootloader/esp_targets.c 
                bootloader/example_common.c 
                # bootloader/md5_hash.c 
                bootloader/protocol_common.c 
                bootloader/protocol_uart.c	
                bootloader/slip.c
                # esp32_iap_sdmmc.c
                http_download.c)

relative_glob(srcs include/*.h src/*.c inc/*.h bootloader/*.c bootloader/inc/*.h) 
beautify_c_code(${target} ${srcs})
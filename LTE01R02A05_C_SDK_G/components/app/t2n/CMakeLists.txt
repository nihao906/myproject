# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#

set(target t2n)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
# include_directories(${SOURCE_TOP_DIR}/components/app/t2n/include)
target_include_directories(${target} PUBLIC include)
# include_directories(${SOURCE_TOP_DIR}/components/app/lis3dsh)
include_directories(${SOURCE_TOP_DIR}/components/app/http_fota/inc)
target_link_libraries(${target} PUBLIC)
target_link_libraries(${target} PRIVATE  esp32_flash http_fota )
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libq_usbnet.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_usbnet.a)
target_sources(${target} PUBLIC
	modbus-tcp.c
	modbus-udp.c
	ModbusS.c
	cJSON.c
	cfg.c
	md5.c
	network_ql.c
	t2n.c
	nmea_tcp.c	
	udp_ota_shell.c
	# lis3dsh_tcp.c
	# BL0939_tcp.c
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
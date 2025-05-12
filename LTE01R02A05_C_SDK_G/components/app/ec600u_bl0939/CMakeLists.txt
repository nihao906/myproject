# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#

# set(target ql_app_t2n)

# add_library(${target} STATIC)
# set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
# target_include_directories(${target} PUBLIC inc)
# #target_link_libraries(${target} PRIVATE  ql_api_common)

# target_sources(${target} 
# 	# cfg.c
# 	# cJSON.c
# 	# md5.c
# 	modbus-tcp.c
# 	modbus-udp.c
# 	ModbusS.c
# 	network_ql.c
# 	t2n.c
# )


# target_sources_if(QL_APP_FEATURE_KEYPAD THEN ${target} PRIVATE keypad_demo.c)
# target_sources_if(QL_APP_FEATURE_RS485	THEN ${target} PRIVATE rs485_demo.c)

# relative_glob(srcs include/*.h src/*.c inc/*.h)
# beautify_c_code(${target} ${srcs})


set(target ec600u_bl0939)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC inc)
include_directories(${SOURCE_TOP_DIR}/components/app/t2n/include)
target_link_libraries(${target} PUBLIC)
target_link_libraries(${target} PRIVATE  rtk)
target_link_libraries(${target} PRIVATE  t2n)

target_sources(${target} PUBLIC
	ac_current.c
	app_can.c
	mcp2515.c
	# spi_demo.c
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
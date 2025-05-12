
set(target modbus_rtu_slave)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC inc)
include_directories(${SOURCE_TOP_DIR}/components/app/t2n/include)
target_link_libraries(${target} PUBLIC)
target_link_libraries(${target} PRIVATE  esp32_flash)

target_sources(${target} PUBLIC
	modbus_slave.c
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
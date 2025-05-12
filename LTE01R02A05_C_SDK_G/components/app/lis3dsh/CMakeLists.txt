
set(target lis3dsh)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PRIVATE fft_lib/inc)
include_directories(${SOURCE_TOP_DIR}/components/app/t2n/include)
# target_link_libraries(${target} PUBLIC)
target_link_libraries(${target} PRIVATE  t2n)

target_sources(${target} PUBLIC
	lis3dsh_intterupt.c
	lis3dsh_reg.c
	./fft_lib/dsps_fft2r_fc32_ansi.c
	./fft_lib/dsps_pwroftwo.c
	./fft_lib/dsps_wind_hann_f32.c
	./fft_lib/dsps_fft2r_bitrev_tables_fc32.c
	json_file_ops.c
)

relative_glob(srcs include/*.h src/*.c inc/*.h fft_lib/inc/*.h)
beautify_c_code(${target} ${srcs})
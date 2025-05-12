set(target lvgl_app)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC inc)
target_include_directories(${target} PUBLIC ${SOURCE_TOP_DIR}/components/app/lvgl_lib/lvgl)
target_include_directories(${target} PUBLIC ${SOURCE_TOP_DIR}/components/app/uart1)
target_link_libraries(${target} PRIVATE lvgl_lib)
target_sources(${target} PUBLIC
	lvgl_demo.c
	./ui/ui.c
	./ui/ui_screen1.c
	./ui/ui_screen2.c
	./ui/ui_screen3.c
	./ui/ui_screen3_chart.c
	./ui/ui_screen4.c
	./ui/fs_function.c
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})

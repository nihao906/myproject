set(target lvgl)
#add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_LVGL)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SRC_DIR} lvgl include)
#target_link_libraries(${target} PRIVATE kernel driver hal ql_api_common)

file(GLOB_RECURSE INCLUDES 
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/lv_examples/src/*.h 
    # ${CMAKE_CURRENT_LIST_DIR}/lv_examples/*.h  
    ${CMAKE_CURRENT_LIST_DIR}/lv_drivers/src/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/lv_drivers/*.h   ./*.h
)
file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/*.c 
    ${CMAKE_CURRENT_LIST_DIR}/lv_examples/src/*.c 
    ${CMAKE_CURRENT_LIST_DIR}/lv_drivers/src/*.c 
    ./main/src/main.c 
    ./main/src/ls_keyboard.c
    ./main/src/lv_port_indev.c
    ./main/src/GT911.c
    ./main/src/MyI2C.c
)

target_sources(${target} PUBLIC
	${SOURCES}
	./main/src/main.c 
)
relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})

 
# file(GLOB_RECURSE INCLUDES "lv_drivers/*.h" "lv_examples/*.h"  "lvgl/*.h"  "./*.h")
# file(GLOB_RECURSE SOURCES  "lv_drivers/*.c" "lv_examples/*.c"  "lvgl/*.c")
# add_library(${target} STATIC ${SOURCES} ${INCLUDES})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
# target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_ZTL)
# target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

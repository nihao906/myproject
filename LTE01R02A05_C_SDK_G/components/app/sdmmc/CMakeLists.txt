set(target sdmmc)    

add_library(${target} STATIC) 
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir}) 
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC) 
target_include_directories(${target} PUBLIC ./ inc)

target_sources(${target} 
                PUBLIC     
                sdmmc.c
                )

relative_glob(srcs *.h inc/*.h) 
beautify_c_code(${target} ${srcs})
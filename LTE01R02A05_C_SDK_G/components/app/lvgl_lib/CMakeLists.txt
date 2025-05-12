# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

#configure_file(include/lv_gui_config.h.in ${out_inc_dir}/lv_gui_config.h)

set(target lvgl_lib)
#add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_LVGL)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SRC_DIR} lvgl include)

target_sources(${target} PRIVATE
    # core
    lvgl/src/core/lv_disp.c
    lvgl/src/core/lv_event.c
    lvgl/src/core/lv_group.c
    lvgl/src/core/lv_indev_scroll.c
    lvgl/src/core/lv_indev.c
    lvgl/src/core/lv_obj_class.c
    lvgl/src/core/lv_obj_draw.c
    lvgl/src/core/lv_obj_pos.c
    lvgl/src/core/lv_obj_scroll.c
    lvgl/src/core/lv_obj_style_gen.c
    lvgl/src/core/lv_obj_style.c
    lvgl/src/core/lv_obj_tree.c
    lvgl/src/core/lv_obj.c
    lvgl/src/core/lv_refr.c
    lvgl/src/core/lv_theme.c
    # draw
    # lvgl/src/draw/nxp_pxp/lv_gpu_nxp_pxp_osa.c
    # lvgl/src/draw/nxp_pxp/lv_gpu_nxp_pxp.c
    # lvgl/src/draw/nxp_vglite/lv_gpu_nxp_vglite.c
    # lvgl/src/draw/sdl/lv_draw_sdl_arc.c
    # lvgl/src/draw/sdl/lv_draw_sdl_bg.c
    # lvgl/src/draw/sdl/lv_draw_sdl_composite.c
    # lvgl/src/draw/sdl/lv_draw_sdl_img.c
    # lvgl/src/draw/sdl/lv_draw_sdl_label.c
    # lvgl/src/draw/sdl/lv_draw_sdl_line.c
    # lvgl/src/draw/sdl/lv_draw_sdl_mask.c
    # lvgl/src/draw/sdl/lv_draw_sdl_polygon.c
    # lvgl/src/draw/sdl/lv_draw_sdl_rect.c
    # lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c
    # lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c
    # lvgl/src/draw/sdl/lv_draw_sdl_utils.c
    # lvgl/src/draw/sdl/lv_draw_sdl.c
    # lvgl/src/draw/stm32_dma2d/lv_gpu_stm32_dma2d.c
    # lvgl/src/draw/sw/lv_draw_sw.c
    # lvgl/src/draw/sw/lv_draw_sw_arc.c
    # lvgl/src/draw/sw/lv_draw_sw_blend.c
    # lvgl/src/draw/sw/lv_draw_sw_dither.c
    # lvgl/src/draw/sw/lv_draw_sw_gradient.c
    # lvgl/src/draw/sw/lv_draw_sw_img.c
    # lvgl/src/draw/sw/lv_draw_sw_letter.c
    # lvgl/src/draw/sw/lv_draw_sw_line.c
    # lvgl/src/draw/sw/lv_draw_sw_polygon.c
    # lvgl/src/draw/sw/lv_draw_sw_rect.c
    lvgl/src/draw/lv_draw_arc.c
    lvgl/src/draw/lv_draw_blend.c
    lvgl/src/draw/lv_draw_img.c
    lvgl/src/draw/lv_draw_label.c
    lvgl/src/draw/lv_draw_line.c
    lvgl/src/draw/lv_draw_mask.c
    lvgl/src/draw/lv_draw_rect.c
    lvgl/src/draw/lv_draw_triangle.c
    # lvgl/src/draw/lv_draw.c
    lvgl/src/draw/lv_img_buf.c
    lvgl/src/draw/lv_img_cache.c
    lvgl/src/draw/lv_img_decoder.c
    # extra
    lvgl/src/extra/layouts/flex/lv_flex.c
    lvgl/src/extra/layouts/grid/lv_grid.c
    lvgl/src/extra/libs/bmp/lv_bmp.c
    # lvgl/src/extra/libs/ffmpeg/lv_ffmpeg.c
    lvgl/src/extra/libs/freetype/lv_freetype.c
    lvgl/src/extra/libs/fsdrv/lv_fs_fatfs.c
    lvgl/src/extra/libs/fsdrv/lv_fs_posix.c
    lvgl/src/extra/libs/fsdrv/lv_fs_stdio.c
    lvgl/src/extra/libs/fsdrv/lv_fs_win32.c
    lvgl/src/extra/libs/gif/gifdec.c
    lvgl/src/extra/libs/gif/lv_gif.c
    lvgl/src/extra/libs/png/lodepng.c
    lvgl/src/extra/libs/png/lv_png.c
    lvgl/src/extra/libs/qrcode/lv_qrcode.c
    lvgl/src/extra/libs/qrcode/qrcodegen.c
    lvgl/src/extra/libs/rlottie/lv_rlottie.c
    lvgl/src/extra/libs/sjpg/lv_sjpg.c
    lvgl/src/extra/libs/sjpg/tjpgd.c
    # lvgl/src/extra/others/gridnav/lv_gridnav.c
    # lvgl/src/extra/others/monkey/lv_monkey.c
    lvgl/src/extra/others/snapshot/lv_snapshot.c
    lvgl/src/extra/themes/basic/lv_theme_basic.c
    lvgl/src/extra/themes/default/lv_theme_default.c
    lvgl/src/extra/themes/mono/lv_theme_mono.c
    lvgl/src/extra/widgets/animimg/lv_animimg.c
    lvgl/src/extra/widgets/calendar/lv_calendar_header_arrow.c
    lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.c
    lvgl/src/extra/widgets/calendar/lv_calendar.c
    lvgl/src/extra/widgets/chart/lv_chart.c
    lvgl/src/extra/widgets/colorwheel/lv_colorwheel.c
    lvgl/src/extra/widgets/imgbtn/lv_imgbtn.c
    lvgl/src/extra/widgets/keyboard/lv_keyboard.c
    lvgl/src/extra/widgets/led/lv_led.c
    lvgl/src/extra/widgets/list/lv_list.c
    lvgl/src/extra/widgets/meter/lv_meter.c
    lvgl/src/extra/widgets/msgbox/lv_msgbox.c
    # lvgl/src/extra/widgets/menu/lv_menu.c
    lvgl/src/extra/widgets/span/lv_span.c
    lvgl/src/extra/widgets/spinbox/lv_spinbox.c
    lvgl/src/extra/widgets/spinner/lv_spinner.c
    lvgl/src/extra/widgets/tabview/lv_tabview.c
    lvgl/src/extra/widgets/tileview/lv_tileview.c
    lvgl/src/extra/widgets/win/lv_win.c
    lvgl/src/extra/lv_extra.c
    # font
    lvgl/src/font/lv_font.c
    lvgl/src/font/lv_font_fmt_txt.c
    lvgl/src/font/lv_font_loader.c
    # lvgl/src/font/lv_font_montserrat_8.c
    # lvgl/src/font/lv_font_montserrat_10.c
    # lvgl/src/font/lv_font_montserrat_12_subpx.c
    lvgl/src/font/lv_font_montserrat_14.c
    # lvgl/src/font/lv_font_montserrat_18.c
    # lvgl/src/font/lv_font_montserrat_22.c
    lvgl/src/font/lv_font_montserrat_24.c
    # lvgl/src/font/lv_font_montserrat_26.c
    # lvgl/src/font/lv_font_montserrat_28_compressed.c
    # lvgl/src/font/lv_font_montserrat_30.c
    # lvgl/src/font/lv_font_montserrat_32.c
    # lvgl/src/font/lv_font_montserrat_34.c
    # lvgl/src/font/lv_font_montserrat_36.c
    # lvgl/src/font/lv_font_montserrat_38.c
    # lvgl/src/font/lv_font_montserrat_40.c
    # lvgl/src/font/lv_font_montserrat_44.c
    # lvgl/src/font/lv_font_montserrat_46.c
    # lvgl/src/font/lv_font_montserrat_48.c
    lvgl/src/font/ui_screen1_font_16.c
    lvgl/src/font/system_status_icon_24.c
    lvgl/src/font/lv_font_montserrat_12.c
    lvgl/src/font/lv_font_montserrat_16.c
    lvgl/src/font/lv_font_montserrat_20.c
    lvgl/src/font/lv_font_montserrat_42.c
    lvgl/src/font/ui_screen2_font_16.c
    lvgl/src/font/lv_font_chinese_12_name.c
    lvgl/src/font/ui_font_12.c
    lvgl/src/font/ui_font_16.c
    # gpu
    # hal
    lvgl/src/hal/lv_hal_disp.c
    lvgl/src/hal/lv_hal_indev.c
    lvgl/src/hal/lv_hal_tick.c
    # misc
    lvgl/src/misc/lv_anim_timeline.c
    lvgl/src/misc/lv_anim.c
    lvgl/src/misc/lv_area.c
    lvgl/src/misc/lv_async.c
    lvgl/src/misc/lv_bidi.c
    lvgl/src/misc/lv_color.c
    lvgl/src/misc/lv_fs.c
    lvgl/src/misc/lv_gc.c
    lvgl/src/misc/lv_ll.c
    lvgl/src/misc/lv_log.c
    # lvgl/src/misc/lv_lru.c
    lvgl/src/misc/lv_math.c
    lvgl/src/misc/lv_mem.c
    lvgl/src/misc/lv_printf.c
    lvgl/src/misc/lv_style_gen.c
    lvgl/src/misc/lv_style.c
    lvgl/src/misc/lv_templ.c
    lvgl/src/misc/lv_timer.c
    lvgl/src/misc/lv_tlsf.c
    lvgl/src/misc/lv_txt_ap.c
    lvgl/src/misc/lv_txt.c
    lvgl/src/misc/lv_utils.c
    # widgets
    lvgl/src/widgets/lv_arc.c
    lvgl/src/widgets/lv_bar.c
    lvgl/src/widgets/lv_btn.c
    lvgl/src/widgets/lv_btnmatrix.c
    lvgl/src/widgets/lv_canvas.c
    lvgl/src/widgets/lv_checkbox.c
    lvgl/src/widgets/lv_dropdown.c
    lvgl/src/widgets/lv_img.c
    lvgl/src/widgets/lv_label.c
    lvgl/src/widgets/lv_line.c
    lvgl/src/widgets/lv_objx_templ.c
    lvgl/src/widgets/lv_roller.c
    lvgl/src/widgets/lv_slider.c
    lvgl/src/widgets/lv_switch.c
    lvgl/src/widgets/lv_table.c
    lvgl/src/widgets/lv_textarea.c
)
relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
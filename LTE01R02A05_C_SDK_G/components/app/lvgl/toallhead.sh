#!/bin/bash

lvgl="./lvgl/src/"
lvgl_drv="./lv_drivers/display/"

echo "find all file..."
lvgl_head_all=$(find $lvgl -name "*.h")
lvgl_drv_head_all=$(find $lvgl_drv -name "*.h")

LUA_HEAD_PATH="./lua_head/"
mkdir -p $LUA_HEAD_PATH

echo "cat lvgl all file..."
lvgl_head_all_path="$LUA_HEAD_PATH/lua_lvgl_all.h"
for i in $lvgl_head_all; do
	cat $i >>$lvgl_head_all_path
done
cat $lvgl/../lvgl.h >>$lvgl_head_all_path

echo "cat lvgl drv all file..."
lvgl_drv_head_all_path="$LUA_HEAD_PATH/lua_drv_lvgl_all.h"
for i in $lvgl_drv_head_all; do
	cat $i >>$lvgl_drv_head_all_path
done
cat $lvgl_drv/../win_drv.h >>$lvgl_drv_head_all_path

echo "done."
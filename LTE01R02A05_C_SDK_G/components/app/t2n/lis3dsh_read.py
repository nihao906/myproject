# -*- coding: utf-8 -*-
import socket
import struct
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
import threading
import time
from hexdump import hexdump
from collections import deque
import os


# 设置TCP连接参数
HOST = '11.5.6.80'  # 你的数据源IP地址
PORT = 9998        # 你的数据源端口号
TIMEOUT = 5.0
RAW_DATA_CNT = 1024
FEQ_DATA_CNT = 1024 // 2

# 用于存储接收到的数据
rawdata  = deque([deque(maxlen=RAW_DATA_CNT), deque(maxlen=RAW_DATA_CNT), deque(maxlen=RAW_DATA_CNT)])
feq_data = deque([deque(maxlen=FEQ_DATA_CNT), deque(maxlen=FEQ_DATA_CNT), deque(maxlen=FEQ_DATA_CNT)])
feq_min = 0
feq_max = 0
a_max = 0
is_shake = 0

# # 锁定图形绘制，防止多线程冲突
# lock = threading.Lock()

feq_read_ok_flag = False


def delete_if_exists(file_name):
    if os.path.exists(file_name):
        try:
            os.remove(file_name)
            print("File", file_name, "deleted successfully.")
        except Exception as e:
            print("An error occurred while deleting the file:", e)
    else:
        print("File", file_name, "does not exist.")

def append_floats_to_file(file_name, float_list):
    try:
        # 拼接文件路径
        # 以追加模式打开文件，如果文件不存在则创建
        with open(file_name, 'a+') as f:
            # 将浮点数列表逐行写入文件
            i = 0
            for num in float_list:
                f.write(str(i) + ":" + str(num) + '\n')
                i += 1
        print("Data appended successfully to", file_name)
    except Exception as e:
        print("An error occurred:", e)


# 读取TCP数据并解析
def read_tcp_data():
    global feq_read_ok_flag
    global feq_min
    global feq_max
    global a_max
    global is_shake

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(TIMEOUT)
        s.connect((HOST, PORT))
        try:
            while True:
                # 接收数据
                buf = s.recv(4)
                if len(buf) != 4:
                    continue
                data_id = buf[1]
                data_len = buf[2] | buf[3] << 8
                if buf[0] == 0xE1:  # 原始数据包
                    print("raw data_id:" + str(data_id))
                    if data_id < 0 or data_id >= 3:
                        continue
                    if data_len != RAW_DATA_CNT * 4:
                        continue
                    # buf_old = b''
                    while data_len > 0:
                        buf = s.recv(data_len // 4 * 4)
                        # buf = s.recv(data_len)
                        # buf = buf_old + buf
                        # buf_old = b''
                        if len(buf) <= 0:
                            break
                        # if len(buf) % 4:
                        #     buf_old = buf[-len(buf) % 4:]
                        #     buf = buf[:-len(buf) % 4]
                        # if len(buf) == 0:
                        #     continue
                        # print(len(buf))
                        # print(len(buf) // 4)
                        if len(buf) % 4 != 0:
                            break
                        float_data = struct.unpack('f' * (len(buf) // 4), buf)
                        data_len -= len(buf)
                        # with lock:
                        # if data_id == 1:
                        #     float_data = [x + 1 for x in float_data]
                        rawdata[data_id].extend(float_data)
                        append_floats_to_file(r"C:\Users\Administrator\data_out" + r"\rawdata" + str(data_id) + ".txt", float_data)
                elif buf[0] == 0xE2:  # 频谱图 必须是整体放进去才可以
                    print("feq data_id:" + str(data_id))
                    if data_id < 0 or data_id >= 3:
                        continue
                    if data_len != FEQ_DATA_CNT * 4:
                        continue
                    feq_read_ok_flag = False
                    # buf_old = []
                    # buf_old = b''
                    while data_len > 0:
                        # buf = s.recv(data_len // 4 * 4)
                        buf = s.recv(data_len)
                        # buf = buf_old + buf
                        # buf_old = b''
                        if len(buf) <= 0:
                            break
                        # if len(buf) % 4:
                        #     buf_old = buf[-len(buf) % 4:]
                        #     buf = buf[:-len(buf) % 4]
                        if len(buf) % 4 != 0:
                            feq_data[data_id] = []
                            break
                        float_data = struct.unpack('f' * (len(buf) // 4), buf)
                        data_len -= len(buf)
                        # with lock:
                        feq_data[data_id].extend(float_data)
                        append_floats_to_file(r"C:\Users\Administrator\data_out" + r"\feqdata" + str(data_id) + ".txt", float_data)
                        # if data_id == 1:
                        #     print(float_data)
                    feq_read_ok_flag = True
                elif buf[0] == 0xE3:  # shake判断的有关数据
                    print("shake data_id:" + str(data_id))
                    if data_id != 0:
                        continue
                    if data_len != 14:
                        continue
                    buf = s.recv(data_len)
                    if len(buf) != data_len:
                        continue
                    feq_min, feq_max, a_max, is_shake = struct.unpack('fffh', buf)
                    print(feq_min, feq_max, a_max, is_shake)

        except socket.timeout:
            print("timeout")
        except Exception as e:
            print(f"err:{e}")

threading.Thread(target=read_tcp_data).start()



# 创建图形和初始线条
fig, (ax1, ax2) = plt.subplots(1, 2)
line1, = ax1.plot([], 'r-', label='x')
line2, = ax1.plot([], 'g-', label='y')
line3, = ax1.plot([], 'b-', label='z')
ax1.legend()
ax1.set_ylim(-1, 1)
ax1.set_xlim(0, RAW_DATA_CNT)

line4, = ax2.plot([], 'r-', label='ch1-y')
line5, = ax2.plot([], 'g-', label='ch2-z')
line6, = ax2.plot([], 'b-', label='sum')
ax2.legend()
ax2.set_ylim(0, 0.5)
ax2.set_xlim(0, FEQ_DATA_CNT)

feq_min_line = ax2.axvline(x=feq_min, color='gray', linestyle='--') 
feq_max_line = ax2.axvline(x=feq_max, color='gray', linestyle='--') 
a_max_line = ax2.axhline(y=a_max, color='brown', linestyle='--')

text = ax2.text(0, 0, 'is_shake:' + str(is_shake), fontsize=12, color='red', ha='center', va='center')


def update(frame):
    global feq_read_ok_flag
    line1.set_data(range(len(rawdata[0])), rawdata[0])
    line2.set_data(range(len(rawdata[1])), rawdata[1])
    line3.set_data(range(len(rawdata[2])), rawdata[2])

    if feq_read_ok_flag:
        line4.set_data(range(len(feq_data[0])), feq_data[0])
        line5.set_data(range(len(feq_data[1])), feq_data[1])
        line6.set_data(range(len(feq_data[2])), feq_data[2])
    
    feq_min_line.set_xdata(feq_min)
    feq_max_line.set_xdata(feq_max)
    a_max_line.set_ydata(a_max)

    text.set_text('is_shake:' + str(is_shake))

    return line1, line2, line3, line4, line5, line6, feq_min_line, feq_max_line, a_max_line, text

ani = FuncAnimation(fig, update, frames=None, interval=100)
plt.show()
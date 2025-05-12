

def append_to_file(file_path, content):
    try:
        # 以追加模式打开文件，如果文件不存在则创建
        with open(file_path, 'a+') as f:
            # 写入内容到文件末尾
            f.write(content)
        print("Content appended successfully to", file_path)
    except Exception as e:
        print("An error occurred:", e)

# 测试示例
file_path = r'C:\Users\Administrator\data_out\b.txt' 
content = "This is some new content.\n"  # 要追加的内容
append_to_file(file_path, content)
#!/bin/bash

echo "开始清理和构建项目..."

# 清理构建目录
rm -rf build

# 创建构建目录并进入
mkdir build && cd build

# 运行CMake配置
cmake ..

# 编译项目
make

# 安装（需要sudo权限）
sudo make install

# 更新动态链接库缓存
sudo ldconfig

echo "构建和安装完成！"

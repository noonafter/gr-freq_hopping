# gr-freq_hopping

基于 GNU Radio 的跳频通信系统模块

## 项目简介

gr-freq_hopping 是一个基于 GNU Radio 框架开发的跳频通信系统 OOT (Out-of-Tree) 模块。该模块实现了完整的跳频通信链路，包括时隙帧生成、PSK 基带调制、频率跳变、解跳、符号恢复和性能测量等功能。

### 主要特性

- **完整的发射链路**：从信息生成到频跳调制的完整处理流程
- **完整的接收链路**：从解跳到符号恢复和误码率测量
- **灵活的跳频速率**：支持 5/10/20/50/100/110 hops/s
- **多种调制方式**：支持 BPSK/QPSK/8PSK
- **格雷码编码**：降低符号错误率
- **RRC 成形滤波**：改善频谱效率
- **高效的同步机制**：基于标签传播的时间和相位同步
- **性能测量**：集成符号错误率（SER）测量功能

## 系统架构

### 发射链路
```
slot_frame → bb_pskmod → hop_interp → hop_mod → RF发射
   (帧生成)   (PSK调制)    (插值)    (频跳调制)
```

### 接收链路
```
RF接收 → hop_demod → symbol_recover → frame_recover → ser_measurement
        (频跳解调)   (符号恢复)        (帧恢复)        (误码率测量)
```

## 模块说明

### 发射端模块

#### 1. slot_frame（时隙帧生成）
- 生成包含同步头和数据段的时隙帧结构
- 支持源信息生成与加扰
- 输出长度随跳频速率动态调整：
  - 5 hops/s: 432 符号
  - 10 hops/s: 216 符号
  - 20 hops/s: 108 符号
  - 50 hops/s: 40 符号
  - 100 hops/s: 20 符号
  - 110 hops/s: 18 符号

**参数**：
- `hop_rate`: 跳频速率（hops/s）
- `M_order`: PSK调制阶数（2/4/8）
- `info_seed`: 信息序列随机种子

#### 2. bb_pskmod（PSK 基带调制）
- PSK 星座映射，支持 BPSK/QPSK/8PSK
- 使用格雷码编码
- RRC 根升余弦滤波与成形
- 可配置过采样因子（2x/4x/8x/16x）

**参数**：
- `hop_rate`: 跳频速率
- `M_order`: 调制阶数（2/4/8）
- `Ksa_ch`: 每符号采样点数（过采样因子）

#### 3. hop_interp（插值）
- 对复数信号进行插值处理
- 支持向量化信号流

**参数**：
- `interp_fac`: 插值因子
- `vlen_in`: 输入向量长度

#### 4. hop_mod（频跳调制）
- 基于伪随机序列的频率跳变
- 使用液体 DSP 库的 NCO 实现高效频率转换
- 自动处理 110 跳特殊速率（9600/87 Hz）

**参数**：
- `bw_hop`: 跳频带宽
- `ch_sep`: 信道间隔
- `freq_carrier`: 载波频率
- `fsa_hop`: 跳频采样率
- `hop_rate`: 跳频速率

### 接收端模块

#### 5. hop_demod（频跳解调）
- 与发射端同步生成跳频序列
- 基于 rx_time 标签实现时间同步
- 通过负频率混频实现解跳

**参数**：与 hop_mod 相同

#### 6. symbol_recover（符号恢复）
- 监听 `phase_est` 标签实现采样同步
- 基于标签值进行相位补偿
- 抽取最佳采样点
- 过滤假标签（基于相关峰值判断）

**参数**：
- `sps`: 每符号采样点数

#### 7. frame_recover（帧恢复）
- 基于 `phase_est` 标签进行帧同步
- 在帧长度内输出数据
- 跳频切换期间不输出符号

**参数**：
- `frame_len`: 帧长度

#### 8. ser_measurement（符号错误率测量）
- 与参考文件对比计算符号错误率
- 每 30 帧输出一次统计结果
- 稀疏输出模式（仅在帧边界）

**参数**：
- `filename`: 参考符号序列文件路径

## 依赖项

### 必需依赖
- **GNU Radio** >= 3.10
- **liquid-dsp**：液体 DSP 信号处理库
- **CMake** >= 3.8
- **Python** >= 3.6
- **pybind11**：C++/Python 绑定

### 系统库
- gnuradio-runtime
- gnuradio-filter
- C++ 标准库（complex, vector, random, fstream 等）

## 编译安装

### 1. 安装依赖

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install gnuradio gnuradio-dev cmake python3-dev pybind11-dev

# 安装 liquid-dsp
git clone https://github.com/jgaeddert/liquid-dsp.git
cd liquid-dsp
./bootstrap.sh
./configure
make
sudo make install
sudo ldconfig
```

#### Fedora/RHEL
```bash
sudo dnf install gnuradio gnuradio-devel cmake python3-devel pybind11-devel
# liquid-dsp 需要从源码编译
```

### 2. 编译模块

```bash
cd gr-freq_hopping
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```

### 3. 验证安装

在 Python 中测试导入：
```python
python3 -c "import gnuradio.freq_hopping; print('安装成功！')"
```

在 GNU Radio Companion 中，应该能看到 "freq_hopping" 分类下的所有模块。

## 使用方法

### 在 GNU Radio Companion 中使用

1. 打开 GNU Radio Companion (gnuradio-companion)
2. 在右侧模块列表中找到 "freq_hopping" 分类
3. 将所需的模块拖拽到流图画布
4. 配置各模块的参数
5. 连接模块构建完整的发射或接收链路

### 典型流图示例

#### 发射端流图
```
freq_hopping_slot_frame
    ↓
freq_hopping_bb_pskmod
    ↓
freq_hopping_hop_interp
    ↓
freq_hopping_hop_mod
    ↓
UHD: USRP Sink (或其他硬件设备)
```

#### 接收端流图
```
UHD: USRP Source (或其他硬件设备)
    ↓
freq_hopping_hop_demod
    ↓
resampler
    ↓
agc2
    ↓
correlation estimator
    ↓
freq_hopping_symbol_recover
    ↓
freq_hopping_frame_recover
    ↓
costas_loop
    ↓
constellation decoder
    ↓
freq_hopping_ser_measurement
```

### Python 辅助函数

模块提供了一些辅助计算函数：

```python
from gnuradio import freq_hopping

# 计算 slot_frame 输出向量长度
vlen = freq_hopping.calc_vlen_slot_frame(hop_rate=110)

# 计算 bb_pskmod 输出向量长度
vlen = freq_hopping.calc_vlen_bb_pskmod(hop_rate=110)

# 计算 9600 波特率模式的头部长度
head_len = freq_hopping.calc_head_len_9600(hop_rate=110)
```

## 技术特性

### 标签传播机制
- 大多数模块设置标签传播策略为 `TPP_DONT`
- 通过手动 `add_item_tag` 实现精细的标签控制
- 关键同步标签：`phase_est`、`corr_est`、`rx_time`

### 同步机制
- **时间同步**：基于 rx_time 标签
- **相位同步**：基于 phase_est 标签
- **帧同步**：基于 corr_est 标签和相关峰检测

### 特殊处理
- **110 跳速率**：使用 9600/87 = 110.34 Hz 实际跳频率以实现收发同步
- **向量化处理**：支持 vlen 参数进行批量数据处理
- **液体 DSP 集成**：使用 liquid 库的 NCO 进行高效频率转换

## 文件结构

```
gr-freq_hopping/
├── include/gnuradio/freq_hopping/  # 公开 API 头文件
├── lib/                             # C++ 实现源文件
├── python/freq_hopping/             # Python 模块和绑定
├── grc/                             # GRC 块配置文件
├── examples/                        # 示例流图（待添加）
├── apps/                            # 应用程序（待添加）
├── docs/                            # 文档
└── CMakeLists.txt                   # 构建配置
```

## 开发与测试

### 运行单元测试
```bash
cd build
ctest -V
```

### 代码格式化
项目包含 `.clang-format` 配置文件，可以使用以下命令格式化代码：
```bash
find . -name "*.cc" -o -name "*.h" | xargs clang-format -i
```

## 贡献

欢迎提交 Issue 和 Pull Request！
#
# Copyright 2008,2009 Free Software Foundation, Inc.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# The presence of this file turns this directory into a Python package

'''
This is the GNU Radio FREQ_HOPPING module. Place your Python package
description here (python/__init__.py).
'''
import os

# import pybind11 generated symbols into the freq_hopping namespace
try:
    # this might fail if the module is python-only
    from .freq_hopping_python import *
except ModuleNotFoundError:
    pass

def calc_vlen_slot_frame(hop_rate):
    mapping = {5:432, 10:216, 20:108, 50:40, 100:20, 110:18}
    return mapping.get(hop_rate, 432)

def calc_vlen_bb_pskmod(hop_rate):
    """
    计算 bb_pskmod 块的输出向量长度

    参数:
    hop_rate: 跳频速率
    Ksa_ch: 过采样因子

    返回:
    输出向量长度
    """
    # 根据 hop_rate 确定基础长度
    base_mapping = {
        5: 480*4,    # 200 * Ksa_ch
        10: 240*4,   # 100 * Ksa_ch
        20: 120*4,    # 50 * Ksa_ch
        50: 48*4,    # 20 * Ksa_ch
        100: 24*4,   # 10 * Ksa_ch
        110: 87     # 9 * Ksa_ch
    }

    base_length = base_mapping.get(hop_rate, 480*4)  # 默认 200 * Ksa_ch
    return base_length
# import any pure python here
#

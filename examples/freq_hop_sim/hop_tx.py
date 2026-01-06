#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Freqency Hopping Transmitter
# Author: chuan li
# GNU Radio version: 3.10.1.1

from packaging.version import Version as StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from gnuradio import blocks
from gnuradio import freq_hopping
from gnuradio.freq_hopping import calc_vlen_slot_frame
from gnuradio.freq_hopping import calc_vlen_bb_pskmod
from gnuradio.freq_hopping import calc_head_len_9600
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
from gnuradio.qtgui import Range, RangeWidget
from PyQt5 import QtCore
import numpy as np



from gnuradio import qtgui

class hop_tx(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Freqency Hopping Transmitter", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Freqency Hopping Transmitter")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "hop_tx")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.hop_rate = hop_rate = 20
        self.tx_gain = tx_gain = 30
        self.samp_rate = samp_rate = 2457600
        self.num_sym1hop = num_sym1hop = calc_vlen_slot_frame(hop_rate)
        self.num_sample1hop = num_sample1hop = calc_vlen_bb_pskmod(hop_rate)
        self.interp_factor = interp_factor = 256
        self.cnt_dir = cnt_dir = '/home/lc/桌面/gnuradio_proj/oot_module/'
        self.M_order = M_order = 4

        ##################################################
        # Blocks
        ##################################################
        self._tx_gain_range = Range(0, 60, 1, 30, 100)
        self._tx_gain_win = RangeWidget(self._tx_gain_range, self.set_tx_gain, "transmit power gain", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_layout.addWidget(self._tx_gain_win)
        self.uhd_usrp_sink_0_0 = uhd.usrp_sink(
            ",".join(("serial=326C056", '')),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
            "",
        )
        self.uhd_usrp_sink_0_0.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_0_0.set_time_now(uhd.time_spec(time.time()), uhd.ALL_MBOARDS)

        self.uhd_usrp_sink_0_0.set_center_freq(70e6, 0)
        self.uhd_usrp_sink_0_0.set_antenna("TX/RX", 0)
        self.uhd_usrp_sink_0_0.set_gain(tx_gain, 0)
        self.freq_hopping_slot_frame_0 = freq_hopping.slot_frame(hop_rate, M_order, 12345)
        self.freq_hopping_hop_mod_0 = freq_hopping.hop_mod(1e6, 3e3, 500e3, samp_rate, hop_rate, num_sample1hop*interp_factor)
        self.freq_hopping_hop_interp_0 = freq_hopping.hop_interp(interp_factor, num_sample1hop)
        self.freq_hopping_bb_pskmod_0 = freq_hopping.bb_pskmod(hop_rate, M_order, 4)
        self.blocks_vector_to_stream_0_0_0_0 = blocks.vector_to_stream(gr.sizeof_gr_complex*1, num_sample1hop*interp_factor)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_vector_to_stream_0_0_0_0, 0), (self.uhd_usrp_sink_0_0, 0))
        self.connect((self.freq_hopping_bb_pskmod_0, 0), (self.freq_hopping_hop_interp_0, 0))
        self.connect((self.freq_hopping_hop_interp_0, 0), (self.freq_hopping_hop_mod_0, 0))
        self.connect((self.freq_hopping_hop_mod_0, 0), (self.blocks_vector_to_stream_0_0_0_0, 0))
        self.connect((self.freq_hopping_slot_frame_0, 0), (self.freq_hopping_bb_pskmod_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "hop_tx")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_hop_rate(self):
        return self.hop_rate

    def set_hop_rate(self, hop_rate):
        self.hop_rate = hop_rate
        self.set_num_sample1hop(calc_vlen_bb_pskmod(self.hop_rate))
        self.set_num_sym1hop(calc_vlen_slot_frame(self.hop_rate))

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain
        self.uhd_usrp_sink_0_0.set_gain(self.tx_gain, 0)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_sink_0_0.set_samp_rate(self.samp_rate)

    def get_num_sym1hop(self):
        return self.num_sym1hop

    def set_num_sym1hop(self, num_sym1hop):
        self.num_sym1hop = num_sym1hop

    def get_num_sample1hop(self):
        return self.num_sample1hop

    def set_num_sample1hop(self, num_sample1hop):
        self.num_sample1hop = num_sample1hop

    def get_interp_factor(self):
        return self.interp_factor

    def set_interp_factor(self, interp_factor):
        self.interp_factor = interp_factor

    def get_cnt_dir(self):
        return self.cnt_dir

    def set_cnt_dir(self, cnt_dir):
        self.cnt_dir = cnt_dir

    def get_M_order(self):
        return self.M_order

    def set_M_order(self, M_order):
        self.M_order = M_order




def main(top_block_cls=hop_tx, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()

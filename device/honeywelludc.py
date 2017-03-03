#!/usr/bin/env python

"""
Honeywell UDC 3x00 device communicating through Modbus RTU via RS-485
"""

import time
import minimalmodbus
from goserver.constants import GoStatus, GoOperation, GoTransactionId
from goserver.common import FlowVar
from .deviceobj import DeviceBase, DeviceVar

__author__ = 'Yun Hua'
__email__ = 'huayunfly@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'
__date__ = '2017.03.03'


class HoneywellUDC(DeviceBase):
    """
    Honeywell UDC 3200, 3300 device.
    """

    def __init__(self, name, service, port, address):
        super().__init__(name, service)
        var = DeviceVar(name='udc_pv1')
        self.vars.append(var)
        var = DeviceVar(name='udc_sp1')
        self.vars.append(var)

        self.port = port
        self.address = address
        self.registers_r = [0x40]
        self.registers_w = [0x78]
        self.instrument = minimalmodbus.Instrument(self.port, self.address)
        self.instrument.handle_local_echo = False
        self.instrument.debug = False
        self.invalid_value = -27648.0

    def refresh_worker(self):
        while self.refreshing:
            names = []
            indexes = []
            op_results = []
            values = []
            for i, var in enumerate(self.vars):
                if i < len(self.registers_r):
                    names.append(var.name)
                    indexes.append(i)
                    try:
                        value = self.instrument.read_float(
                            self.registers_r[i], functioncode=4, numberOfRegisters=2)
                    except ValueError:
                        print('UDC read float value error')
                        values.append(self.invalid_value)
                        op_results.append(GoStatus.S_COMM_ERROR)
                    except OSError:
                        print('UDC read float os error')
                        values.append(self.invalid_value)
                        op_results.append(GoStatus.S_COMM_ERROR)
                    else:
                        values.append(value)
                        op_results.append(GoStatus.S_OK)
                else:
                    break
            flow_var = FlowVar(names, indexes, values,
                               GoOperation.OP_REFRESH, op_results, GoTransactionId.ID_EMPTY)

            self.out_queue.put(flow_var)
            time.sleep(0.25)

    def internal_write(self, flow_var):
        results = []
        for i, value in zip(flow_var.indexes, flow_var.values):
            try:
                index_w = i - len(self.registers_r)
                self.instrument.write_float(self.registers_w[index_w], value)
            except ValueError:
                print('UDC write float value error')
                results.append(GoStatus.S_COMM_ERROR)
            except OSError:
                print('UDC write float os error')
                results.append(GoStatus.S_COMM_ERROR)
            else:
                results.append(GoStatus.S_OK)
        flow_var.values = None
        flow_var.op_results = results
        flow_var.op_mode = GoOperation.OP_ASYNC_WRITE
        self.out_queue.put(flow_var)








# Unit test for OMRON E5CC/EC/AC* controller using modbus RTU via RS-485, by FDTI-USB convertor.

import time
import unittest
import minimalmodbus
minimalmodbus.STOPBITS = 2
minimalmodbus.TIMEOUT = 0.1

__author__ = 'Yun Hua'
__date__ = '2017.11.06'


class OmronE5XCTest(unittest.TestCase):
    """
    Test cases for OMRON E5CC/EC/AC* controller communication through Modbus via RS485
    On E5CC/EC/AC*, modbus protocal communication choices:
        BYTESIZE = 8 (fixed)
        PARITY = EVEN or ODD -> STOPBITS = 1
        PARITY = NONE -> STOPBITS = 2

    Using 4 bytes variable mode in the test cases. One variable uses 4 bytes, aka 2 Modbus registers.
    (2 bytes mode is optional.)

    The controller needs > 0.1s corresponding time (minimalmodubs.TIMEOUT) while writing SP.
    """
    def setUp(self):
        self.port = "/dev/tty.usbserial-DN00N126"
        self.address = 2
        self.register_pv = 0x0000
        self.register_sp = 0x0106
        self.instrument = minimalmodbus.Instrument(self.port, self.address)
        self.instrument.handle_local_echo = False
        self.instrument.debug = False
        self.close_port_after_each_call = False
        self.minimal_interval_after_write_sp = 0.1

    def tearDown(self):
        pass

    def test_read_pv(self):
        """
        Test read PV, read_registers() returns an int list, e.g. [0, 211] means 21.1 degC
        """
        for i in range(5):
            temperature = self.instrument.read_registers(
                self.register_pv, numberOfRegisters=2, functioncode=3)
            print('Current temperature: {}'.format(temperature))

    def test_write_sp(self):
        """
        Test write SP, write_register() send an int list, e.g. [0, 105] means 10.5 degC
        Warning: an integer must not exceed range 0x7FFF (32767, means 3276.7 degC).
                Considering the range of thermal couple K, the first one of the int list
                shall always be 0.
        """
        for i in range(10):
            self.instrument.write_registers(self.register_sp, [0, 105 + i])
            # After set SP, a minimal interval must be waited
            # before the next command send (Read PV or Write SP),
            # for the instrument needs time to handle setting SP.
            time.sleep(self.minimal_interval_after_write_sp)


if __name__ == '__main__':
    unittest.main()

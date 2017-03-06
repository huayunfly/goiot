# Unit test for modbus RTU via RS-485, using FDTI USB convertor

import unittest
import minimalmodbus

__author__ = "Yun Hua"

class ModbusUSB4UDCTest(unittest.TestCase):
    """
    Unit test for MODBUS RTU via RS-485. The master is HONEYWELL UDC3200, 3300
    """
    def setUp(self):
        self.port = "/dev/tty.usbserial-DN00N126"
        self.address = 2
        self.register_pv = 0x40
        self.register_sp = 0x78
        self.instrument = minimalmodbus.Instrument(self.port, self.address)
        self.instrument.handle_local_echo = False
        self.instrument.debug = False
        self.close_port_after_each_call = False

    def tearDown(self):
        self.instrument.serial.close()

    def test_read_pv(self):
        for i in range(100):
            temperature = self.instrument.read_float(
                self.register_pv, functioncode=4, numberOfRegisters=2)
            print('Current temperature: {}'.format(temperature))

    def test_write_sp(self):
        with self.assertRaises(OSError) as e:
            self.instrument.write_float(self.register_sp, 120.0)
        print('OSError: {}'.format(e.exception))


if __name__ == '__main__':
    unittest.main()

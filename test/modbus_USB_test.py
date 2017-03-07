# Unit test for modbus RTU via RS-485, using FDTI USB convertor

import time
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
        self.minimal_interval_after_write_sp = 0.21

    def tearDown(self):
        #self.instrument.serial.close()
        pass

    def test_read_pv(self):
        for i in range(50):
            temperature = self.instrument.read_float(
                self.register_pv, functioncode=4, numberOfRegisters=2)
            print('Current temperature: {}'.format(temperature))

    def test_write_sp(self):
        for i in range(10):
            self.instrument.write_float(self.register_sp, float(i))
            # After set UDC SP, a minimal interval must be waited
            # before the next command send (Read PV or Write SP),
            # for the instrument needs time to handle setting SP.
            time.sleep(self.minimal_interval_after_write_sp)


if __name__ == '__main__':
    unittest.main()

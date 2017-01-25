#!/usr/bin/env python

"""
@summary: Access UDC-3200 via modbus-RTU on serial
@author: Yun Hua, yun_hua@yashentech.com
@date: 2017.01.04
"""

from device import minimalmodbus
import RPi.GPIO as GPIO
import time

RPI_RS485_RSE_PIN = 12
UDC3200_LOOP1_SP_ADDR = 0x78
UDC3200_LOOP1_PV_ADDR = 0x40

# Reference: http://stackoverflow.com/questions/17081442/python-modbus-library
if __name__ == '__main__':
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(RPI_RS485_RSE_PIN, GPIO.OUT)
    
    # UDC3200 address 2, baud 19200, loop1 PV address 0x40,
    instrument = minimalmodbus.Instrument('/dev/ttyS0', 2)
    instrument.handle_local_echo = False
    instrument.debug = False

    while True:
        try:
            # loop1 SP address 0x78
            # Write loop1 Set Point command: 0x02 10 00 78 00 02 04 42 c8 00 00 6f ef
            # Read loop1 PV command: 0x02 04 00 40 00 02 70 2c
            temperature = instrument.read_float(
                UDC3200_LOOP1_PV_ADDR, functioncode=4, numberOfRegisters=2)
            print('Current temperature: {}'.format(temperature))
            time.sleep(1.0)
            #instrument.write_float(UDC3200_LOOP1_SP_ADDR, 11.9)
        except ValueError as e:
            print(e)
            time.sleep(1.0)
        except OSError as e:
            print(e)
            print('OSError........................')
            time.sleep(1.0)
            
    GPIO.cleanup()
    print('Test over.')


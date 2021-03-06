# _*_ coding: utf-8 _*_
# !/usr/bin/env python

"""
@summary: Initialize DB
@author: Yun Hua, yun_hua@yashentech.com
@date: 2017.01.04
"""

import os
from datetime import datetime
import django


if __name__ == "__main__":
    os.environ.setdefault(
        "DJANGO_SETTINGS_MODULE",
        "goiot.settings"
    )

    # Removes all data from the database
    from django.core.management import execute_from_command_line

    execute_from_command_line(['', 'flush'])

    # Avoid error while importing User: django.core.exceptions.AppRegistryNotReady: Apps aren't loaded yet.
    django.setup()

    from dataserver.models import DataSource
    import pytz

    # UDC device
    d1 = DataSource(id=1, name='udc_pv1', summary='udc3200_pv', dvalue=10.1, dtype='FLOAT',
                    direction='IN', device='UDC', device_no=1, dcommand='PV', hlimit=200.0,
                    llimit=0.0, deadband=0.01, refresh_rate=100.0, last_update=datetime.now(pytz.timezone("Asia/Shanghai")))

    d2 = DataSource(id=2, name='udc_sp1', summary='udc3200_sp', dvalue=20.0, dtype='FLOAT',
                    direction='INOUT', device='UDC', device_no=1, dcommand='SP', hlimit=200.0,
                    llimit=0.0, deadband=0.01, refresh_rate=100.0, last_update=datetime.now(pytz.timezone("Asia/Shanghai")))

    d1.save()
    d2.save()

    # mockup device
    for i in range(5):
        di = DataSource(id=i+3, name='Sim_'+str(i), summary='mockup_sim_'+str(i), dvalue=float(i), dtype='FLOAT',
                        direction='IN', device='mockup', device_no=1, dcommand='PV', hlimit=10000.0,
                        llimit=0.0, deadband=0.01, refresh_rate=100.0,
                        last_update=datetime.now(pytz.timezone("Asia/Shanghai")))
        di.save()

    i = 8
    di = DataSource(id=i, name='Sim_' + str(5), summary='mockup_sim_' + str(5), dvalue=float(i), dtype='FLOAT',
                    direction='INOUT', device='mockup', device_no=1, dcommand='SP', hlimit=10000.0,
                    llimit=0.0, deadband=0.01, refresh_rate=100.0,
                    last_update=datetime.now(pytz.timezone("Asia/Shanghai")))
    di.save()

    print('Initialized GoIoT dataserver DB.')







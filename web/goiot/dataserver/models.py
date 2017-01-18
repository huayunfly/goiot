"""
@summary: Database definition
@author: Yun Hua, yun_hua@yashentech.com
@date: 2017.01.04
"""

from __future__ import unicode_literals

from django.db import models


class DataSource(models.Model):
    id = models.IntegerField(primary_key=True)
    name = models.CharField(max_length=120)
    summary = models.CharField(max_length=254)
    dvalue = models.FloatField()
    dtype = models.CharField(max_length=120)
    direction = models.CharField(max_length=60)
    device = models.CharField(max_length=120)
    device_no = models.IntegerField()
    dcommand = models.CharField(max_length=120)
    hlimit = models.FloatField(null=True)
    llimit = models.FloatField(null=True)
    deadband = models.FloatField(null=True)
    refresh_rate = models.FloatField()
    last_update = models.DateTimeField()






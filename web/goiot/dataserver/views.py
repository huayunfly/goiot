# -*- coding: utf-8 -*-

"""
@summary: View definition
@author: Yun Hua, yun_hua@yashentech.com
@date: 2017.01.04
"""

from datetime import datetime
from django.shortcuts import render
from django.http import HttpRequest
from django.http import JsonResponse

from dataserver.models import DataSource

DA_Q_KEY = 'key'


def home(request):
    """Renders the home page."""
    assert isinstance(request, HttpRequest)
    return render(
        request,
        'dataserver/index.html',
        {
            'title': 'Home Page',
            'year': datetime.now().year,
        }
    )


def da_api(request):
    """Renders the data access APIs"""
    assert isinstance(request, HttpRequest)

    if request.is_ajax():
        key = request.GET.get(DA_Q_KEY)
        if key is not None:
            return JsonResponse(
                {'data': [d.dvalue for d in DataSource.objects.filter(id=key)]}
            )
        else:
            return JsonResponse({'data', 'null'})

    else:
        return render(
            request,
            'dataserver/da.html',
            {
                'title': '数据访问',
                'year': datetime.now().year,
            }
        )

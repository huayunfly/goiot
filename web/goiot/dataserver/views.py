# -*- coding: utf-8 -*-

"""
@summary: View definition
@author: Yun Hua, yun_hua@yashentech.com
@date: 2017.01.04
"""

from datetime import datetime
import json
from django.shortcuts import render
from django.http import HttpRequest
from django.http import HttpResponseBadRequest
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt

from dataserver.models import DataSource

DA_Q_DATA = 'data'
DA_Q_TOKEN = 'token'
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


def report(request):
    """Renders the report pages."""
    assert isinstance(request, HttpRequest)
    return render(
        request,
        'dataserver/report.html',
        {
            'title': '报表',
            'year': datetime.now().year,
        }
    )


def about(request):
    """Renders the about pages."""
    assert isinstance(request, HttpRequest)
    return render(
        request,
        'dataserver/about.html',
        {
            'title': '说明',
            'year': datetime.now().year,
        }
    )


@csrf_exempt
def tags_rw(request):
    """Tags access API handler."""
    assert isinstance(request, HttpRequest)

    token = request.GET.get(DA_Q_TOKEN)
    if 'GET' == request.method and (token is not None):
        key_content = request.GET.get(DA_Q_KEY)
        response_data = {'data': []}
        if key_content is None:
            for obj in DataSource.objects.all():
                response_data['data'].append(
                    {'key': obj.id, 'name': obj.name, 'value': obj.dvalue, 'type': obj.dtype,
                     'direction': obj.direction, 'device': obj.device, 'device_no': obj.device_no}
                )
            return JsonResponse(response_data)
        else:
            try:
                key_set = json.loads(key_content)
            except TypeError:
                return HttpResponseBadRequest()
            except json.decoder.JSONDecodeError:
                return HttpResponseBadRequest()
            try:
                keys = key_set['data']
                for item in keys:
                    key = item['key']
                    obj = DataSource.objects.filter(id=key)[0]
                    response_data['data'].append(
                        {'key': obj.id, 'name': obj.name, 'value': obj.dvalue, 'type': obj.dtype,
                         'direction': obj.direction, 'device': obj.device, 'device_no': obj.device_no}
                    )
            except KeyError:
                return HttpResponseBadRequest()
            except IndexError:
                return HttpResponseBadRequest()
            else:
                return JsonResponse(response_data)
    elif 'POST' == request.method:
        raw_data = request.POST.get(DA_Q_DATA)
        if raw_data is not None:
            try:
                data_obj = json.loads(raw_data)
            except TypeError:
                return HttpResponseBadRequest()
            except json.decoder.JSONDecodeError:
                return HttpResponseBadRequest()
            try:
                data = data_obj['data']
                for item in data:
                    key = item['key']
                    value = item['value']
                    data_item = DataSource.objects.filter(id=key)[0]
                    if True:  # 'INOUT' == data_item.direction:
                        f_val = float(value)
                        if f_val < data_item.hlimit:
                            data_item.dvalue = f_val
                            data_item.save()
            except KeyError:
                # Dict key error.
                return HttpResponseBadRequest()
            except IndexError:
                # Data source query error, invalid id.
                return HttpResponseBadRequest()
            except ValueError:
                # Float conversion error.
                return HttpResponseBadRequest()
            else:
                return JsonResponse({'op_result': [0]})
        else:
            return HttpResponseBadRequest('No data.')
    else:
        return render(
            request,
            'dataserver/da.html',
            {
                'title': '数据访问',
                'year': datetime.now().year,
                'dataset': DataSource.objects.all()
            }
        )


def tag_rw(request, tag_id):
    """Tag read or write API handler"""
    assert isinstance(request, HttpRequest)

    if 'POST' == request.method:
        token = request.POST.get(DA_Q_TOKEN)
        assert token is not None

        raw_data = request.POST.get(DA_Q_DATA)
        if raw_data is not None:
            try:
                data_obj = json.loads(raw_data)
            except TypeError:
                return HttpResponseBadRequest()
            except json.decoder.JSONDecodeError:
                return HttpResponseBadRequest()
            try:
                data = data_obj['data']
                for item in data:
                    key = item['key']
                    value = item['value']
                    assert key == int(tag_id)
                    data_item = DataSource.objects.filter(id=key)[0]
                    if 'INOUT' == data_item.direction:
                        f_val = float(value)
                        if f_val < data_item.hlimit:
                            data_item.dvalue = f_val
                            data_item.save()
            except KeyError:
                # Dict key error.
                return HttpResponseBadRequest()
            except IndexError:
                # Data source query error, invalid id.
                return HttpResponseBadRequest()
            except ValueError:
                # Float conversion error.
                return HttpResponseBadRequest()
            else:
                return JsonResponse({'op_result': [0]})
        else:
            return HttpResponseBadRequest('No data.')
    elif 'GET' == request.method:
        token = request.GET.get(DA_Q_TOKEN)
        assert token is not None

        try:
            data_item = DataSource.objects.filter(id=int(tag_id))[0]
        except IndexError:
            return HttpResponseBadRequest()
        else:
            return JsonResponse({'data': [{'key': tag_id, 'value': data_item.dvalue}]})
    else:
        assert False

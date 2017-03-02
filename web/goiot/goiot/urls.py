"""goiot URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/1.10/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  url(r'^$', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  url(r'^$', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.conf.urls import url, include
    2. Add a URL to urlpatterns:  url(r'^blog/', include('blog.urls'))
"""
from django.conf.urls import url
from dataserver import views


urlpatterns = [
    # Uncomment the next line to enable the admin:
    # url(r'^admin/', admin.site.urls),
    url(r'^$', views.home, name='start'),
    url(r'^da/api/tags', views.tags_rw, name='tags_rw'),
    url(r'^da/api/tag/(?P<tag_id>[0-9]{3})', views.tag_rw, name='tag_rw'),
    url(r'^report/', views.report, name='report'),
    url(r'^about/', views.about, name='about'),
]

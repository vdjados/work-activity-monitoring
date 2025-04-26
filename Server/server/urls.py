from django.contrib import admin
from django.urls import path, include
from monitor.views import ClientListView, ClientDetailView
from django.conf import settings
from django.conf.urls.static import static

urlpatterns = [
    path('admin/', admin.site.urls),
    path('monitor/', include('monitor.urls')),
    path('', ClientListView.as_view(), name='client-list'),
    path('client/<int:pk>/', ClientDetailView.as_view(), name='client-detail'),
]

if settings.DEBUG:
    urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
from django.urls import path
from .views import ClientListView, ClientDetailView


urlpatterns = [
    path('', ClientListView.as_view(), name='client-list'),
    path('<int:pk>/', ClientDetailView.as_view(), name='client-detail'),
]

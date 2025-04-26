from django.views import generic
from .models import Client
from logging import getLogger

logger = getLogger('monitor')

class ClientListView(generic.ListView):
    model = Client
    template_name = 'monitor/client_list.html'
    context_object_name = 'clients'

    def get_queryset(self):
        qs = super().get_queryset().order_by('-last_active')
        logger.info('Rendering client list')
        return qs

class ClientDetailView(generic.DetailView):
    model = Client
    template_name = 'monitor/client_detail.html'
    context_object_name = 'client'

    def get(self, request, *args, **kwargs):
        logger.info(f"Rendering detail for client {self.get_object()}")
        return super().get(request, *args, **kwargs)
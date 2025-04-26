from rest_framework import viewsets
from .models import Client
from .serializers import ClientSerializer
from rest_framework.decorators import action
from rest_framework.response import Response

class ClientViewSet(viewsets.ModelViewSet):
    queryset = Client.objects.all()
    serializer_class = ClientSerializer

    @action(detail=True, methods=['post'])
    def upload_screenshot(self, request, pk=None):
        client = self.get_object()
        screenshot = request.FILES.get('screenshot')
        if screenshot:
            client.screenshot = screenshot
            client.save()
            return Response({'status': 'screenshot uploaded'})
        return Response({'error': 'no screenshot provided'}, status=400)

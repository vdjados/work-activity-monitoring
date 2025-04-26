from django.contrib import admin
from .models import Client

@admin.register(Client)
class ClientAdmin(admin.ModelAdmin):
    list_display = ('domain', 'computer_name', 'ip_address', 'username', 'last_active')

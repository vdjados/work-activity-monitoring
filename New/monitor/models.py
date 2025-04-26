from django.db import models

class Client(models.Model):
    domain = models.CharField(max_length=100)
    computer_name = models.CharField(max_length=100)
    ip_address = models.GenericIPAddressField()
    username = models.CharField(max_length=100)
    last_active = models.DateTimeField(auto_now=True)
    screenshot = models.ImageField(upload_to='screenshots/', blank=True, null=True)

    def __str__(self):
        return f'{self.domain}\\{self.username} ({self.ip_address})'

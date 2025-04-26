import os
import django
import socket
import struct
import json
import time

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'server.settings')
django.setup()

from django.utils import timezone
from django.core.files.base import ContentFile
from monitor.models import Client

HOST = '0.0.0.0'
PORT = 5000

def handle_client(conn, addr):
    try:
        # Чтение длины JSON-заголовка
        raw = conn.recv(4)
        if not raw:
            return
        hdr_len = struct.unpack('<i', raw)[0]

        # Чтение JSON
        data = b''
        while len(data) < hdr_len:
            chunk = conn.recv(hdr_len - len(data))
            if not chunk:
                break
            data += chunk
        header = json.loads(data.decode('utf-8'))

        # Чтение длины скриншота
        raw = conn.recv(4)
        scr_len = struct.unpack('<i', raw)[0]

        # Чтение скриншота 
        scr_data = b''
        while len(scr_data) < scr_len:
            chunk = conn.recv(scr_len - len(scr_data))
            if not chunk:
                break
            scr_data += chunk
        # Сохранение в БД
        domain   = header.get('domain', '')
        computer = header.get('machine', '')
        username = header.get('user', '')
        ip_addr  = addr[0]

        client_obj, created = Client.objects.update_or_create(
            domain=domain,
            computer_name=computer,
            username=username,
            ip_address=ip_addr,
            defaults={'last_active': timezone.now()}
        )

        if scr_data:
            ts = int(time.time())
            fname = f"{domain}_{username}_{ts}.bmp"
            client_obj.screenshot.save(fname, ContentFile(scr_data))
            client_obj.save()

        print(f"[{time.asctime()}] Received from {ip_addr}: {domain}\\{username}")

    except Exception as e:
        print("Error handling client:", e)
    finally:
        conn.close()

def run_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((HOST, PORT))
    sock.listen(5)
    print(f"TCP server listening on {HOST}:{PORT} ...")
    try:
        while True:
            conn, addr = sock.accept()
            handle_client(conn, addr)
    except KeyboardInterrupt:
        print("Shutting down.")
    finally:
        sock.close()

if __name__ == '__main__':
    run_server()

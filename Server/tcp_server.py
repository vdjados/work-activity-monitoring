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
        # 1) Чтение длины JSON (4 байта, big-endian)
        raw = conn.recv(4)
        if not raw or len(raw) < 4:
            return
        hdr_len = struct.unpack('!I', raw)[0]

        # 2) Чтение JSON ровно hdr_len байт
        data = b''
        while len(data) < hdr_len:
            chunk = conn.recv(hdr_len - len(data))
            if not chunk:
                break
            data += chunk
        header = json.loads(data.decode('utf-8'))

        # 3) Чтение длины скриншота (4 байта, big-endian)
        raw = conn.recv(4)
        if not raw or len(raw) < 4:
            return
        scr_len = struct.unpack('!I', raw)[0]

        # 4) Чтение scr_len байт чистого бинарника
        scr_data = b''
        while len(scr_data) < scr_len:
            chunk = conn.recv(min(4096, scr_len - len(scr_data)))
            if not chunk:
                break
            scr_data += chunk

        # …далее ваша логика сохранения в БД
        # например:
        client_obj, created = Client.objects.update_or_create(
            domain=header['domain'], computer_name=header['machine'],
            username=header['user'], ip_address=addr[0],
            defaults={'last_active': timezone.now()}
        )
        if scr_data:
            fname = f"{header['domain']}_{header['user']}_{int(time.time())}.bmp"
            client_obj.screenshot.save(fname, ContentFile(scr_data))
            client_obj.save()

        print(f"[{time.asctime()}] From {addr[0]}: {header['domain']}\\{header['user']}")

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

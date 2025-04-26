# Мониторинг рабочей активности

## Сервер

```bash
cd Server
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
python manage.py migrate
start cmd /k "python manage.py runserver"
start cmd /k "python tcp_server.py"
Открыть http://127.0.0.1:8000/
```
## Клиент

Открыть Client/Client.sln в Visual Studio.

Построить и запустить (F5).

Клиент → 127.0.0.1:5000 → TCP сервер → Django DB → веб-UI
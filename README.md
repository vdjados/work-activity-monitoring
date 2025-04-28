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

**Install:**  
   `MyClientService.exe install`

**Start/Stop:**  
   `net start MyClientService`  
   `net stop  MyClientService`
   
**Uninstall:**  
   `MyClientService.exe uninstall`
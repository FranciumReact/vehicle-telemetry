FROM python:3.12-slim
WORKDIR /app
# Dependencies first, so editing app.py doesn't invalidate this cached layer.
COPY dashboard/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY dashboard/ .
CMD ["python", "app.py"]
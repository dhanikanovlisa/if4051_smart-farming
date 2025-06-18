FROM python:3.11-slim

WORKDIR /app

# Copy requirements first for better caching
COPY requirements.txt .

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Copy application code
COPY . .

# Make sure we can see the files
RUN ls -la /app/

# Set Python path and run
ENV PYTHONPATH=/app
ENV PYTHONUNBUFFERED=1

# Run the MQTT listener
CMD ["python", "-u", "/app/main.py"]
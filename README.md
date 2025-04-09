# Smart Farming

## Description
A brief description of the project.

## Prerequisites
- [Docker](https://www.docker.com/) installed on your system.

## Setup

1. Clone the repository:
    ```bash
    git clone <repository-url>
    cd <repository-folder>
    ```

2. Create and configure the `.env` file:
    Copy the example `.env` file and fill in the required values.
    ```bash
    cp .env.example .env
    ```
    Edit the `.env` file:
    ```env
    # Example environment variables
    MQTT_BROKER_IP=mqtt
    MQTT_BROKER_PORT=1883
    ```

3. Build and run the Docker containers:
    ```bash
    docker-compose up --build
    or
    docker-compose up -d
    ```
4. Change your ESP32 IP Address to your devices IP
   ```
   hostname -I //Linux
   ipconfig //Windows
   ```
## Usage
Access the application at `http://localhost:<APP_PORT>`.

## Stopping the Containers
To stop the containers, run:
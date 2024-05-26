
# Health tracking platform for dairy cows

This project consists of an ESP32-based bracelet that monitors and sends simulated health data (temperature and step count) to a server via Bluetooth Low Energy (BLE). The data is encrypted using the ChaCha20 encryption algorithm to ensure secure transmission. A Python script running on a server acts as a proxy, decrypting the received data and forwarding it to a Django server for further processing and storage.

## Workflow


**1. ESP32 Setup:**

   **.** The ESP32 runs the esp.ino code, simulating health data.

   **.** It encrypts this data using the ChaCha20 encryption algorithm.

   **.** The encrypted data is sent to the connected BLE client (Python script) via notifications.


**2. Server Setup:**

   **.** Ensure that Bluetooth is enabled on the server before executing the proxy script.

   **.** Run the Python script (proxy.py) which connects to the ESP32.

   **.** The script decrypts the incoming data and forwards it to the Django server.




## Execution of The project

You need to open the Bluetooth of the server when you run the code 

```bash
  pip install -r requirements.txt
  python proxy.py
```

Additionally, for the esp.ino file, you need to run the code directly on a hardware device that supports BLE.
    

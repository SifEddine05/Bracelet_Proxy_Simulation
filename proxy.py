import asyncio
from bleak import BleakClient
from Crypto.Cipher import ChaCha20
import requests

# Replace this with your ESP32's MAC address
ESP32_MAC_ADDRESS = "30:AE:A4:FE:FF:9E"

# UUIDs for the UART service and characteristics
SERVICE_UUID = "36b29c3e-5fd5-488e-b915-4d04e4d61e7f"
CHARACTERISTIC_UUID_TX = "2d3c4928-5e6f-4f93-9d83-08a278a4b729"

# ChaCha20 key (32 bytes)
KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F'

# ChaCha20 nonce (12 bytes, must match the nonce used in ESP32 code)
NONCE = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'



# Callback function to handle notifications
def notification_handler(sender, data):
    print(f"Received encrypted data: {data}")

    # Decrypt the data using ChaCha20
    decrypted_data = decrypt_chacha20(data)
    print(f"Decrypted data: {decrypted_data}")

    send_to_django_server(str(decrypted_data))

    # send_to_django_server(encrypted_data)

# Decrypt the data using ChaCha20
def decrypt_chacha20(data):
    cipher = ChaCha20.new(key=KEY, nonce=NONCE)
    decrypted_data = cipher.decrypt(data)
    return decrypted_data.decode('utf-8')


# Send encrypted data to Django server
def send_to_django_server(data):
    try : 
        url = "https://easy-swine-wise.ngrok-free.app/api/receive-data/"  # Replace with your Django server URL
        headers = {'Content-Type': 'application/json',
                   'AuthorizationKey' : 'SECRET_KEY'
                   }
        payload = {"data": data}
        response = requests.post(url, json=payload, headers=headers)
        print(f"Data sent to Django server, response status: {response.status_code}")
    except Exception as error :
        print(error)
        pass

async def main():
    async with BleakClient(ESP32_MAC_ADDRESS) as client:
        # Ensure the client is connected
        if not client.is_connected:
            print("Failed to connect to the ESP32")
            return

        print("Connected to the ESP32")

        # Discover services and characteristics
        services = await client.get_services()
        characteristic = None

        # Find the characteristic handle by matching the UUID and checking for notification support
        for service in services:
            for char in service.characteristics:
                if char.uuid == CHARACTERISTIC_UUID_TX and 'notify' in char.properties:
                    characteristic = char
                    break
            if characteristic:
                break

        if not characteristic:
            print("Notification characteristic not found")
            return

        # Start receiving notifications from the TX characteristic using its handle
        await client.start_notify(characteristic, notification_handler)

        # Keep the script running to receive notifications
        print("Waiting for notifications...")
        while True:
            await asyncio.sleep(1)

# Run the main function
asyncio.run(main())
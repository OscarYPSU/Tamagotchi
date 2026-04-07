import asyncio
from bleak import BleakScanner, BleakClient, BleakError 

# Replace this with the ID you found (MAC address or UUID)
ADDRESS = "1C:DB:D4:AE:D4:F5"

# MCU disconnected
def on_disconnect(_):
    print("⚠️ MCU disconnected!")

async def run_scanner():
    print("Scanning for devices...")
    # This will scan for 5 seconds by default
    devices = await BleakScanner.discover()
    
    for d in devices:
        # 'd.address' is the MAC (Win/Linux) or UUID (macOS)
        # 'd.name' is the name the device is broadcasting
        print(f"Device: {d.name} | ID: {d.address}")

async def connect_to_device():
    print(f"Connecting to {ADDRESS}...")
    
    # Using 'async with' ensures the connection closes automatically when done
    async with BleakClient(ADDRESS, disconnected_callback=on_disconnect) as client:
        if client.is_connected:
            print("Successfully connected! ✅")
            
            # This lists everything the device can do
            for service in client.services:
                print(f"\nService: {service}")
                for char in service.characteristics:
                    print(f"  - Characteristic: {char.uuid} ({','.join(char.properties)})")
            
            while client.is_connected:
                # asks user for input
                user_input = input("Enter a message ")
                
                # check to see if MCU is still on
                try:
                    await client.write_gatt_char("1b2c3d4e-5f6a-7b8c-9d0e-1f2a3b4c5d6e", user_input.encode())
                    print("Sent value to MCU\n")
                except BleakError as e:
                    print(f"❌ Write failed (MCU likely disconnected): {e}")
                    break
            else:
                print("MCU disconncted\n")
        else:
            print("Failed to connect. ❌")
        await client.disconnect()
        print("Disconnected from Client\n")
    
if __name__ == "__main__":
    asyncio.run(connect_to_device())
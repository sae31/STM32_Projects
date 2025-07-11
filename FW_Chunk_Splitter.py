import serial
import time
import os

def read_bin_in_chunks(filepath, chunk_size=512):
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            if len(chunk) < chunk_size:
                chunk += bytes([0xFF] * (chunk_size - len(chunk)))
            yield chunk

# Serial config
SERIAL_PORT = 'COM46'      # Change this if needed
BAUD_RATE = 115200

# Get file path from user
bin_path = input("Enter path to .bin file: ").strip().strip('"').replace('\\', '/')

if not os.path.isfile(bin_path):
    print("❌ File not found.")
    exit(1)

# Read all 512-byte chunks
all_chunks = list(read_bin_in_chunks(bin_path))
print(f"✅ Loaded {len(all_chunks)} chunks (512 bytes each).")

# Connect to UART
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
except serial.SerialException as e:
    print(f"❌ Serial port error: {e}")
    exit(1)

# 🔄 Send all chunks once
try:
    for i, chunk in enumerate(all_chunks):
        ser.write(chunk)
        print(f"✅ Sent chunk {i + 1}/{len(all_chunks)}")
        print(chunk.hex())
        time.sleep(10)  # Delay between chunks
    print("✅ All chunks sent. Transmission complete.")
except KeyboardInterrupt:
    print("❌ Transmission interrupted by user.")
finally:
    ser.close()



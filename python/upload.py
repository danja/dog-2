#!/usr/bin/env python3
"""
DOG-2 Program Uploader

Uploads programs to DOG-2 via serial connection.
Supports both legacy .txt format and .hex format.
"""
import argparse
import sys
import os.path
import serial
import time


def auto_detect_port():
    """Auto-detect the serial port"""
    possible_ports = ['/dev/ttyUSB0', '/dev/ttyUSB1', '/dev/ttyACM0']
    for port in possible_ports:
        if os.path.exists(port):
            return port
    return None


def read_txt_format(filename):
    """Read legacy .txt format (first 2 chars of each line)"""
    data = ""
    with open(filename, "r") as f:
        for line in f:
            data += line[:2].rstrip()
    return data


def read_hex_format(filename):
    """Read .hex format (continuous hex string)"""
    with open(filename, 'r') as f:
        hex_data = f.read().strip()
    # Remove spaces and newlines
    hex_data = hex_data.replace(' ', '').replace('\n', '')
    return hex_data


def validate_hex(data):
    """Validate that data is valid hex"""
    return all(c in '0123456789ABCDEFabcdef' for c in data)


def upload_program(port, input_file, baud_rate=9600, use_markers=False):
    """Upload a DOG-2 program via serial"""

    # Determine file format by extension
    _, ext = os.path.splitext(input_file)

    if ext == '.txt':
        print(f"Reading legacy .txt format from {input_file}...")
        hex_data = read_txt_format(input_file)
        use_markers = True  # Legacy format uses markers
    elif ext == '.hex':
        print(f"Reading .hex format from {input_file}...")
        hex_data = read_hex_format(input_file)
    else:
        # Try to auto-detect
        print(f"Reading {input_file}...")
        try:
            hex_data = read_hex_format(input_file)
        except:
            hex_data = read_txt_format(input_file)

    # Validate hex data
    if not validate_hex(hex_data):
        print("Error: File contains invalid hex characters")
        sys.exit(1)

    # Add markers if needed (legacy format)
    if use_markers:
        hex_data = "<" + hex_data + ">"

    print(f"Uploading {len(hex_data)} characters to {port} at {baud_rate} baud...")
    if len(hex_data) <= 80:
        print(f"Program data: {hex_data}")
    else:
        print(f"Program data: {hex_data[:40]}...{hex_data[-40:]}")

    try:
        # Open serial connection
        ser = serial.Serial(port, baud_rate, timeout=2)
        print(f"Serial port opened")

        # Wait for Arduino to reset
        reset_delay = 8 if baud_rate == 57600 else 2
        print(f"Waiting {reset_delay}s for Arduino to reset...")
        time.sleep(reset_delay)

        # Send the data
        ser.write(hex_data.encode('utf-8'))
        ser.flush()
        print("Data sent")

        # Wait a bit for processing
        time.sleep(1)

        # Read any response
        response_count = 0
        while ser.in_waiting:
            line = ser.readline().decode('ascii', errors='ignore').strip()
            if line:
                print(f"  < {line}")
                response_count += 1

        ser.close()
        print("\nUpload complete!")

        if response_count == 0:
            print("No response from device (this is normal for some configurations)")

        print("\nNext steps:")
        print("1. Press button 4 on the TM1638 to toggle to RUN_MODE")
        print("2. The display should show 'R' for Run mode")
        print("3. The program will start executing")

    except serial.SerialException as e:
        print(f"Error: Could not open serial port {port}")
        print(f"Details: {e}")
        print("\nCommon serial ports:")
        print("  Linux: /dev/ttyUSB0, /dev/ttyACM0")
        print("  Mac: /dev/tty.usbserial-*")
        print("  Windows: COM3, COM4, etc.")
        sys.exit(1)
    except FileNotFoundError:
        print(f"Error: File not found: {input_file}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='DOG-2 Program Uploader - Upload programs via serial',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python upload.py -i dog-code/fibtone.hex
  python upload.py -i dog-code/fibtone.hex -p /dev/ttyUSB0
  python upload.py -i prog.txt -b 57600

File formats:
  .hex - Continuous hex string (e.g., 0000F03C1001...)
  .txt - Legacy format (first 2 hex chars per line)
        """
    )

    parser.add_argument('-i', '--input-file',
                       action="store",
                       dest='input',
                       required=True,
                       help='Input file (.hex or .txt format)')

    parser.add_argument('-p', '--port',
                       action="store",
                       dest='port',
                       help='Serial port (auto-detected if not specified)')

    parser.add_argument('-b', '--baud',
                       action="store",
                       dest='baud',
                       type=int,
                       default=9600,
                       help='Baud rate (default: 9600)')

    args = parser.parse_args()

    # Auto-detect port if not specified
    port = args.port
    if not port:
        port = auto_detect_port()
        if not port:
            print("Error: Could not auto-detect serial port")
            print("Please specify port with -p option")
            sys.exit(1)
        print(f"Auto-detected port: {port}")

    upload_program(port, args.input, args.baud)

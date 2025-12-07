#!/usr/bin/env python3
"""
DOG-2 Program Uploader

Uploads programs to DOG-2 via serial connection.
Supports .dog (assembly source) and .hex (hex data) formats.
"""
import argparse
import sys
import os
import os.path
import serial
import time
import subprocess


def auto_detect_port():
    """Auto-detect the serial port"""
    possible_ports = ['/dev/ttyUSB0', '/dev/ttyUSB1', '/dev/ttyACM0']
    for port in possible_ports:
        if os.path.exists(port):
            return port
    return None


def read_txt_format(filename):
    """Read assembler output .txt format (hex values, one per line)"""
    data = ""
    with open(filename, "r") as f:
        for line in f:
            # Strip comments and whitespace
            line = line.strip()
            # Skip empty lines and lines starting with digits (address markers)
            if line and not line.startswith('0000'):
                # Extract just the hex digits (before any space or comment)
                hex_part = line.split()[0] if line.split() else ''
                data += hex_part
    return data


def read_hex_format(filename):
    """Read .hex format (continuous hex string)"""
    with open(filename, 'r') as f:
        hex_data = f.read().strip()
    # Remove spaces and newlines
    hex_data = hex_data.replace(' ', '').replace('\n', '')
    return hex_data


def assemble_dog_file(dog_file):
    """Assemble a .dog file to .hex format using ass.py

    Returns the path to the generated .hex file
    """
    # Get the directory and base name
    dog_dir = os.path.dirname(dog_file)
    dog_base = os.path.splitext(os.path.basename(dog_file))[0]

    # Generate output path for .hex file
    hex_file = os.path.join(dog_dir, dog_base + '.hex')

    # Get the path to ass.py (in the same directory as this script)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    ass_py = os.path.join(script_dir, 'ass.py')

    # Get the path to src/core.h (one directory up from script_dir)
    project_dir = os.path.dirname(script_dir)
    core_h = os.path.join(project_dir, 'src', 'core.h')

    if not os.path.exists(ass_py):
        print(f"Error: Assembler not found at {ass_py}")
        sys.exit(1)

    if not os.path.exists(core_h):
        print(f"Error: core.h not found at {core_h}")
        sys.exit(1)

    print(f"Assembling {dog_file} to {hex_file}...")

    # Use a temporary file for assembler output
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', suffix='.txt', delete=False) as tmp:
        tmp_file = tmp.name

    try:
        # Run the assembler to create temporary .txt format
        result = subprocess.run(
            [sys.executable, ass_py, '-i', dog_file, '-o', tmp_file, '-s', core_h],
            capture_output=True,
            text=True,
            check=True
        )

        # Convert .txt to .hex format
        txt_data = read_txt_format(tmp_file)

        with open(hex_file, 'w') as f:
            f.write(txt_data + '\n')

        print(f"  Created {hex_file}")

    except subprocess.CalledProcessError as e:
        print(f"Error running assembler: {e}")
        if e.stderr:
            print(e.stderr)
        sys.exit(1)
    finally:
        # Clean up temporary file
        if os.path.exists(tmp_file):
            os.remove(tmp_file)

    return hex_file


def validate_hex(data):
    """Validate that data is valid hex"""
    return all(c in '0123456789ABCDEFabcdef' for c in data)


def upload_program(port, input_file, baud_rate=9600):
    """Upload a DOG-2 program via serial"""

    # Determine file format by extension
    _, ext = os.path.splitext(input_file)

    if ext == '.dog':
        # Assemble .dog file first
        hex_file = assemble_dog_file(input_file)
        print(f"Reading assembled .hex format from {hex_file}...")
        hex_data = read_hex_format(hex_file)
    elif ext == '.hex':
        print(f"Reading .hex format from {input_file}...")
        hex_data = read_hex_format(input_file)
    else:
        print(f"Error: Unsupported file extension '{ext}'")
        print("Supported formats: .dog (assembly), .hex (hex data)")
        sys.exit(1)

    # Validate hex data
    if not validate_hex(hex_data):
        print("Error: File contains invalid hex characters")
        sys.exit(1)

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
  python upload.py -i dog-code/fibtone.dog
  python upload.py -i dog-code/fibtone.hex
  python upload.py -i dog-code/fibtone.hex -p /dev/ttyUSB0

File formats:
  .dog - Assembly source (will be assembled automatically to .hex)
  .hex - Continuous hex string (e.g., 0000F03C1001...)
        """
    )

    parser.add_argument('-i', '--input-file',
                       action="store",
                       dest='input',
                       required=True,
                       help='Input file (.dog or .hex format)')

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

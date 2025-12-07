# Fibonacci Tone Generator Tutorial

This tutorial shows you how to load and run the `fibtone.dog` program on your DOG-2 device.

## What This Program Does

The Fibonacci Tone Generator creates a quasi-musical sequence by:
1. Calculating successive Fibonacci numbers (1, 1, 2, 3, 5, 8, 13, 21...)
2. Using each Fibonacci value (modulo 16) as a musical note
3. Playing these notes in sequence to create an evolving pattern

The sequence starts gently and gradually builds in complexity, creating an interesting mathematical music pattern.

## Prerequisites

- DOG-2 device connected via USB
- Python 3 installed on your computer
- Serial port access (the device typically appears as `/dev/ttyUSB0` on Linux or `COM3` on Windows)

## Loading the Program

### Step 1: Find Your Serial Port

On Linux:
```bash
ls /dev/ttyUSB*
```

On Mac:
```bash
ls /dev/tty.usb*
```

On Windows, check Device Manager for the COM port (e.g., COM3).

### Step 2: Install Python Serial Library

**Option 1: Using your system package manager (recommended)**

```bash
# Debian/Ubuntu
sudo apt install python3-serial

# Fedora/RHEL
sudo dnf install python3-pyserial

# Arch Linux
sudo pacman -S python-pyserial
```

**Option 2: Using a virtual environment (recommended if system packages unavailable)**

```bash
# Create a virtual environment in the project
cd /home/danny/github/dog-2
python3 -m venv venv

# Activate it
source venv/bin/activate  # Linux/Mac
# or: venv\Scripts\activate  # Windows

# Install pyserial
pip install pyserial

# Now you can run the upload script (while venv is activated)
python python/upload.py -i dog-code/fibtone.hex

# When done, deactivate
deactivate
```

**Option 3: Override externally-managed (not recommended)**

```bash
pip install --break-system-packages pyserial
```

### Step 3: Upload the Program

The DOG-2 repository includes an upload script at `python/upload.py`.

**Auto-detect serial port** (easiest):
```bash
python python/upload.py -i dog-code/fibtone.hex
```

**Specify serial port**:
```bash
# Linux/Mac
python python/upload.py -i dog-code/fibtone.hex -p /dev/ttyUSB0

# Windows
python python\upload.py -i dog-code\fibtone.hex -p COM3
```

**Different baud rate** (if needed for older DOG-1 devices):
```bash
python python/upload.py -i dog-code/fibtone.hex -b 57600
```

### Step 4: Run the Program

1. Press button 4 on the TM1638 to toggle from PROG_MODE to RUN_MODE
2. The display should show "R" (Run mode)
3. The program will automatically start executing
4. You should hear tones playing in a Fibonacci-based sequence

## Understanding the Code

Let's break down what the program does:

### Initialization
```assembly
TEMPO 60        ; Set tempo to 60 (moderate speed)
LDAi 01         ; Load 1 into accumulator A
STAa 007F       ; Store at memory location 0x007F (previous Fib)
LDAi 01         ; Load 1 into accumulator A again
STAa 0080       ; Store at memory location 0x0080 (current Fib)
```
This sets up the first two Fibonacci numbers (both 1) in memory.

### Main Loop - Calculate Next Fibonacci
```assembly
LDAa 0080       ; Load current Fibonacci into A
LDBa 007F       ; Load previous Fibonacci into B
ABA             ; Add B to A: A = F(n) + F(n-1) = F(n+1)
```
This calculates the next number in the sequence.

### Update Storage
```assembly
LDBa 0080       ; Load current into B
STBa 007F       ; Store as new previous
STAa 0080       ; Store F(n+1) as new current
```
Shifts the values: current becomes previous, next becomes current.

### Play the Tone
```assembly
PUSHXA          ; Save Fibonacci value to stack
ANDAi 0F        ; A = A mod 16 (keeps note in range 0-15)
LDBi 08         ; Set duration to 8 units
TONEAB          ; Play tone (note from A, duration from B)
REST 04         ; Brief pause between notes
```
Uses the Fibonacci value (modulo 16) as a musical note and plays it.

### Loop Control
```assembly
POPXA           ; Restore original Fibonacci value
CMPAi C0        ; Compare with 192 (our stopping point)
BZC loop        ; Branch back if less than 192
HALT            ; Stop when we've exceeded 192
```
The program continues until the Fibonacci value exceeds 192, then stops.

## The Fibonacci Series in Music

The Fibonacci series appears in many natural patterns and has interesting musical properties:
- **Sequence**: 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144...
- **Musical mapping**: Each value mod 16 gives notes 0-15
- **Pattern**: Creates a quasi-melodic progression that feels organic

The resulting sound is mathematical yet musical, with patterns that repeat and evolve.

## Troubleshooting

**Program doesn't upload:**
- Check that the serial port is correct
- Ensure no other program is using the serial port
- Try unplugging and replugging the USB cable

**No sound:**
- Check that a speaker is connected to pin 9
- Verify the device entered RUN_MODE (display shows "R")
- Try pressing button 4 to toggle modes

**Upload works but program doesn't run:**
- Press button 4 to enter RUN_MODE
- Check serial monitor for any error messages: `pio device monitor`
- Try buttons 0+2 to reload from EEPROM if the program was saved there

## Saving to EEPROM (Optional)

To save the program to EEPROM so it persists across power cycles, modify the hex file to set the `loadToEEPROM` flag by sending special commands, or manually press the button combination on the device after uploading.

## Variations to Try

Once you understand the basic program, try modifying it:
- Change the modulo value (0x0F) to use a different note range
- Adjust the duration (08) for faster/slower notes
- Modify the tempo (60) for different speeds
- Change the stopping condition (C0/192) for longer/shorter sequences

## Next Steps

- Try writing your own programs using different mathematical sequences
- Combine multiple tones or create harmony
- Use the stack operations to create more complex patterns
- Explore the full instruction set in `docs/manual.md` and `docs/opcodes.md`

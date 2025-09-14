# Sisyphus Protocol Documentation

Data is transmitted in **chunks of up to 1024 bytes**.

Commands are sent as strings, fields separated by spaces, terminated by **EOT** (`0x04`) character.

The device responds according to the specific command.

Errors are reported with messages starting with `err`, and successful operations respond with `ack`.

On invalid arguments:
  ```
  err invalid arguments
  ```
On overflow of the command buffer (128 bytes set in [protocol.h](./protocol.h)):
  ```
  err command buffer overflow
  ```

A handy python shell to test things out is [here](./shell.py), it uses `PySerial` wrapped in a backend package [here](./protocol.py) and also requires `rich` for nice color output.

Protocol version is incremented when there is a breaking change, it will NOT get incremented when eg. a new field in [info](#info) is added to the end.

---

## Command List

- [ls](#ls) - List files
- [mv](#mv) - Move (rename) a file
- [push](#push) - Upload a file to the device
- [pull](#pull) - Download a file from the device
- [rm](#rm) - Remove a file
- [info](#info) - Get information about the device and firmware
- [measure](#measure) - Get a color measurement from the device's color sensor
- [play](#play) - Play an audio file
- [reset](#reset) - Reset the device into bootloader


---

## `ls`

List files in the filesystem.

**Usage:**
```
ls
```

**Device response:**

Each file or directory is listed on a separate line:

```
<name> <type> <size>
```

- `<name>` - file or directory name
- `<type>` - `f` (file) or `d` (directory)
- `<size>` - size in bytes (only present for files)

**Example:**
```
readme.txt f 1234
data f 456
folder d
```

Terminated by EOT.

**Errors:**
- `err cannot open root directory`

---

## `mv`

Rename or move a file.

**Usage:**
```
mv <source> <destination>
```

**Device response:**
- `ack` - on success
- `err file not found` - if source doesn't exist
- `err destination exists` - if destination already exists
- `err rename file` - on failure

---

## `push`

Upload a file to the device.

**Usage:**
```
push <filename> <size> <checksum>
```

- `<size>` - file size in bytes
- `<checksum>` - expected CRC32 checksum (unsigned integer)

**Protocol:**

1. After `push` command, device sends:
   ```
   ack
   ```
2. Send the file data in chunks.
   - After each chunk, **wait for device to send** `ack` **before sending the next chunk**.
3. After all bytes are sent:
   - Device calculates CRC32 and verifies it against `<checksum>`.

**Errors:**
- `err open file`
- `err timeout`
- `err write file`
- `err checksum mismatch` (file deleted automatically on checksum mismatch)

---

## `pull`

Download a file from the device.

**Usage:**
```
pull <filename>
```

**Protocol:**

1. Device responds:
   ```
   ack <size> <checksum>
   ```
   - `<size>` - file size in bytes
   - `<checksum>` - CRC32 checksum

2. Host must immediately reply:
   ```
   ack
   ```

3. Device starts sending the file data in chunks.

4. After each chunk:
   - Host must send `ack` back to device to receive the next chunk.

5. Host should verify the received data against the expected checksum.

**Errors:**
- `err cannot open file`
- `err calculate crc`
- `err timeout`

---

## `rm`

Remove a file.

**Usage:**
```
rm <filename>
```

**Device response:**
- `ack` - if deleted successfully
- `err file not found`
- `err delete file` - if deletion failed

---

## `info`

Lists information about the device.

**Usage:**
```
info
```

**Device response:**
```
<protocol> <device name> <git commit sha> <protocol version> <build date> <block count> <free block count> <block size> <uses eternity> <debug mode>
```

- `<protocol>` - Always `sisyphus`
- `<device name>` - Name of the device set by CMakeLists.txt
- `<git commit sha>` - Git commit SHA at compile time, when compiled from a repository with uncommitted changes `-dirty` gets appended behind it
- `<protocol version>` - Version of the protocol set in [protocol.h](./protocol.h)
- `<build date>` - Date and time of the firmware build in `YYYY-MM-DD,HH:MM:SS`
- `<block count>` - Number of blocks taken up by LittleFS - total size set in CMakeLists.txt
- `<free block count>` - Number of free/unused blocks in LittleFS
- `<block size>` - Size of each block of LittleFS
- `<uses eternity>` - If the FW was built with Eternity's bootloader
- `<debug mode>` - Shows if debug mode is on, if developing a user-friendly frontend make it VERY clear that this is NOT `true` so devs don't accidentally ship a debug device.

Terminated by a newline.

Example:

```
sisyphus 7b82af1-dirty 1 2025-05-05,00:45:02 256 2 4096 1
```

---

## `measure`

Get a color measurement from the device's color sensor. These are the actual values used for color detection with all color processing included.

**Usage:**
```
measure
```

**Device response:**
```
<hue> <saturation> <value> <clear>
```

- `<hue>` - 0-360 - hue in degrees
- `<saturation>` - 0-100 - saturation as a percentage
- `<value>` - 0-100 - value as a percentage
- `<clear>` - 0-255 - brightness in a 8 bit range

Terminated by a newline.

Example:

```
4 56 33 17
```

---

## `play`

Play an audio file stored on the device.

**Usage:**
```
play <filename>
```

**Response:**
- `ack` - if file played successfully
- `err file not found` - if the file does not exist

---

## `reset`

Resets the device into either the bootrom or Eternity, based on CMake configuration.

**Usage:**

```
reset
```

**Device response:**

* None (device resets and reboots into USB bootloader)

---

# Sisyphus Protocol Documentation

Data is transmitted in **chunks of up to 1024 bytes**.

Commands are sent as strings, fields separated by spaces, terminated by **EOT** (`0x04`) character.

The device responds according to the specific command.

Errors are reported with messages starting with `err`, and successful operations respond with `ack`.

---

## Command List

- [ls](#ls) – List files
- [mv](#mv) – Move (rename) a file
- [push](#push) – Upload a file to the device
- [pull](#pull) – Download a file from the device
- [rm](#rm) – Remove a file
- [info](#info) – (Not implemented currently)

---

## `ls`

**List files in the filesystem.**

**Usage:**
```
ls
```

**Device response:**

Each file or directory is listed on a separate line:

```
<name> <type> <size>
```

- `<name>` – file or directory name
- `<type>` – `f` (file) or `d` (directory)
- `<size>` – size in bytes (only present for files)

**Example:**
```
readme.txt f 1234
data f 456
folder d
```

Finished by EOT.

**Errors:**
- If root directory cannot be opened:
  `err cannot open root directory`

---

## `mv`

**Rename or move a file.**

**Usage:**
```
mv <source> <destination>
```

**Device response:**
- `ack` – if the operation succeeded
- `err file not found` – if the source file does not exist
- `err destination exists` – if the destination already exists
- `err rename file` – if renaming failed

---

## `push`

**Upload a file to the device.**

**Usage:**
```
push <filename> <size> <checksum>
```

- `<size>` – file size in bytes
- `<checksum>` – expected CRC32 checksum (unsigned integer)

**Protocol:**

1. After `push` command, device sends:
   ```
   ack
   ```
2. Send the file data in chunks.
   - After each chunk, **wait for device to send** `ack` **before sending the next chunk**.
3. After all bytes are sent:
   - Device calculates CRC32 and verifies it against `<checksum>`.
   - If matching, device responds:
     ```
     ack
     ```
   - Otherwise:
     ```
     err checksum mismatch
     ```

**Errors during transfer:**
- `err open file`
- `err timeout`
- `err write file`
- `err checksum mismatch` (file deleted automatically on checksum mismatch)

---

## `pull`

**Download a file from the device.**

**Usage:**
```
pull <filename>
```

**Protocol:**

1. Device responds:
   ```
   ack <size> <checksum>
   ```
   - `<size>` – file size in bytes
   - `<checksum>` – CRC32 checksum

2. Host must immediately reply:
   ```
   ack
   ```

3. Device starts sending the file data in chunks.

4. After each chunk:
   - Host must send `ack` back to device to receive the next chunk.

**Errors:**
- `err cannot open file`
- `err calculate crc`
- `err timeout`

---

## `rm`

**Remove a file.**

**Usage:**
```
rm <filename>
```

**Device response:**
- `ack` – if file deleted successfully
- `err file not found` – if the file does not exist
- `err delete file` – if deletion failed

---

## `info`

> **Note:** This command is **listed** but **currently not implemented** in source code.

---

# Notes

- Commands are terminated by **EOT** (`0x04`) character.
- Errors always start with `err`.
- On invalid arguments, device responds:
  ```
  err invalid arguments
  ```
- Device uses a **command buffer** internally and will reset if overflowed:
  ```
  err command buffer overflow
  ```

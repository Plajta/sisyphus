# WIP Protocol docs

Data is sent in chunks of up to 1024 bytes. While the device is busy, the client should wait for `EOT` before sending more data.

If multiple "fields" are required, they should be separated by a space.

List of commands:
- [ls](#ls) - List file
- [mv](#mv) - Move file
- [push](#push) - Push file to device
- [pull](#pull) - Pull file from device
- [rm](#rm) - Remove file
- [info](#info) - Get information about device

## ls
List files in the filesystem.

Usage: `ls`

Device will respond with with newline separated list of files terminated by `EOT` like this:
```
<name> <type> <size>
file1 f 123
foo f 332
crazy_dir d
EOT
```
Files are represented by `f` and directories by `d`. Files are required to be listed with their size in bytes.

## mv
Move a file.

Usage: `mv [source] [destination]`

Device will respond `OK` or `ERR` based on success of the operation.
`ERR` should not actually happen with `mv` as it doesn't actually move data around.

## push
Push a file to the device.

Usage: `push [name] [size] [checksum]`

After receiving `ack` from the device, send the file data. After every chunk wait for `ack` from the device. The last chunk should not be padded. After receiving the last chunk the device will respond with `OK` or `ERR` based on success of matching the checksum.

## pull
Pull a file from the device.

Usage: `pull [name]`

After sending expect: `ack <size> <checksum>` from the device after which the file data.

After every chunk the client should send `ack` to the device. The last chunk should not be padded.

## rm
Remove a file.

Usage: `rm [name]`

Device will respond with `OK` or `ERR` based on success of the operation.
`ERR` should not actually happen with `rm` as it just deletes data.

## info
Get information about the device.

Usage: `info`

Device will respond with newline separated info about the device itself.

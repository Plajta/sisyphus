# WIP Protocol docs

Data is sent in chunks of up to 1024 bytes. While the device is busy, the client should wait for the `ack` before sending more data.

If multiple "fields" are required, they should be separated by a space.

List of commands:
- [ls](#ls) - List file
- [mv](#mv) - Move file
- [push](#push) - Push file to device
- [pull](#pull) - Pull file from device
- [rm](#rm) - Remove file
- [info](#info) - Get information about device

Every command has to get answered with `ack` followed by potential data.

## ls
List files in the filesystem.

Usage: `ls`

Device will respond with `ack` followed by a list of files.

## mv
Move a file.

Usage: `mv [source] [destination]`

Device will respond with `ack` followed by a `OK` or `ERR` based on success of the operation.
`ERR` should not actually happen with `mv` as it doesn't actually move data around.

## push
Push a file to the device.

Usage: `push [name] [size] [checksum]`

After receiving `ack` from the device, send the file data. After every chunk wait for `ack` from the device. The last chunk should not be padded. After receiving the last chunk the device will respond with `ack` with a `OK` or `ERR` based on success of matching the checksum.

## pull
Pull a file from the device.

Usage: `pull [name] [size] [checksum]`

After receiving `ack` from the device, expect the file data. After every chunk send `ack` to the device. The last chunk should not be padded.

## rm
Remove a file.

Usage: `rm [name]`

Device will respond with `ack` followed by a `OK` or `ERR` based on success of the operation.
`ERR` should not actually happen with `rm` as it just deletes data.

## info
Get information about the device.

Usage: `info`

Device will respond with `ack` followed by data about the device. The exact data is still being decided.

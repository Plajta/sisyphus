# Sisyphus firmware
This is a prototype firmware that plays sound from it's LittleFS filesystem.


The files can be updated using a USB serial connection. Protocol is described in [Protocol README](./protocol/README.md).
Reference python shell for communication using the protocol is under [protocol/client.py](./protocol/client.py) and uses [protocol/protocol.py](./protocol/protocol.py) as the backend.

By default plays "audio.wav" on button press.

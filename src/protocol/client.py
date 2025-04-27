#!/usr/bin/env python3
import cmd
import readline
import glob
import os
import sys
import time
import zlib
import serial
from rich.console import Console

# Constants
CHUNK_SIZE = 1024
EOT = b'\x04'

console = Console()

# CRC32 helper
def crc32(data, crc=0):
    return zlib.crc32(data, crc) & 0xFFFFFFFF

class DeviceClient:
    def __init__(self, port, baudrate=115200):
        self.serial = serial.Serial(port, baudrate, timeout=1)

    def send_command(self, cmd):
        if isinstance(cmd, str):
            cmd = cmd.encode('ascii')
        self.serial.write(cmd + EOT)
        self.serial.flush()

    def readline(self):
        line = self.serial.readline()
        return line.decode(errors='ignore').strip()

    def ls(self, output=False):
        """List remote files. Returns list of filenames if False; otherwise prints."""

        self.send_command('ls')

        data = bytearray()
        while True:
            byte = self.serial.read(1)
            if byte == EOT or not byte:
                break
            data.extend(byte)
        text = data.decode('ascii', errors='ignore')

        files = text.splitlines()[2:]

        if output:
            console.print("[red]<Filename>[/red] [green]<Size in bytes>[/green]")

        remote_files = []
        for file in files:
            file = file.split(" ")
            remote_files.append(file[0])
            if output:
                console.print(f"[red]{file[0]}[/red] [green]{file[2]}[/green]")
        return remote_files

class DeviceShell(cmd.Cmd):
    prompt = "\033[1;36mSisyphus>\033[0m "

    def __init__(self, device):
        super().__init__()
        self.device = device
        # cache remote files for completion
        self.remote_files = self.device.ls(False)

    def preloop(self):
        readline.parse_and_bind('tab: complete')

    def do_ls(self, arg):
        "ls                List remote files"
        self.remote_files = self.device.ls(True)

    def complete_ls(self, text, line, begidx, endidx):
        return []  # no args

    def do_rm(self, arg):
        "rm <filename>     Remove remote file"
        if not arg:
            console.print("[bold red]Usage: rm <filename>[/bold red]")
        elif arg not in self.remote_files:
            console.print(f"[bold red]No such file: {arg}[/bold red]")
        else:
            self.device.send_command(f"rm {arg}")
            resp = self.device.readline()
            if not resp.startswith('ack'):
                console.print(f"[bold red]Error from device: {resp}[/bold red]")
            else:
                console.print(f"[green]File [bold]{arg}[/bold] removed")
            self.remote_files = self.device.ls(False)

    def complete_rm(self, text, line, begidx, endidx):
        return [f for f in self.remote_files if f.startswith(text)]

    def do_mv(self, arg):
        "mv <old> <new>    Rename remote file"

        parts = arg.split()
        if len(parts) != 2:
            console.print("[bold red]Usage: mv <old> <new>[/bold red]")
            return
        old, new = parts

        if old not in self.remote_files:
            console.print(f"[bold red]No such file: {old}[/bold red]")
            return

        self.device.send_command(f"mv {old} {new}")
        resp = self.device.readline()
        if not resp.startswith('ack'):
            console.print(f"[bold red]Error from device: {resp}[/bold red]")
        else:
            console.print(f"[green]File [bold]{old}[/bold] moved to [bold]{new}[/bold]")

        self.remote_files = self.device.ls(False)

    def complete_mv(self, text, line, begidx, endidx):
        args = line.split()
        if len(args) == 2:
            return [f for f in self.remote_files if f.startswith(text)]
        return []

    def do_push(self, arg):
        "push <local> [remote]   Upload file to device"

        parts = arg.split()
        if not parts:
            console.print("[bold red]Usage: push <local> [remote][/bold red]")
            return
        local = parts[0]
        remote = parts[1] if len(parts)==2 else os.path.basename(local)

        if not os.path.isfile(local):
            console.print(f"[bold red]Local file not found: {local}[/bold red]")
            return
        size = os.path.getsize(local)

        with open(local,'rb') as f:
            checksum = crc32(f.read())

        self.device.send_command(f"push {remote} {size} {checksum}")

        console.print(f"[yellow]Uploading {local} as {remote} ({size} bytes)...[/yellow]")

        sent = 0
        start = time.time()
        with open(local,'rb') as f:
            while sent < size:
                if not self.device.readline().startswith('ack'):
                    console.print("[bold red]No ack, aborting[/bold red]")
                    return

                chunk = f.read(CHUNK_SIZE)
                self.device.serial.write(chunk)
                sent += len(chunk)

                # Show transfer speed
                elapsed = time.time() - start
                speed = sent / elapsed / 1024  # in KB/s
                progress = (sent / size) * 100
                console.print(f"[yellow]Progress: {progress:.1f}% - {speed:.1f} KB/s[/yellow]")

        resp = self.device.readline()
        if not resp.startswith('ack'):
            console.print(f"[bold red]Error from device: {resp}[/bold red]")
        else:
            console.print(f"[green]File [bold]{local}[/bold] uploaded to device")
        self.remote_files = self.device.ls(False)

    def complete_push(self, text, line, begidx, endidx):
        parts = line.split()

        if len(parts) <= 2:
            return glob.glob(text+'*')
        elif len(parts) == 3:
            return [f for f in self.remote_files if f.startswith(text)]
        return []

    def do_pull(self, arg):
        "pull <remote> [local]  Download file from device"

        parts = arg.split()
        if not parts:
            console.print("[bold red]Usage: pull <remote> [local][/bold red]")
            return
        remote = parts[0]
        local = parts[1] if len(parts)==2 else remote

        self.device.send_command(f"pull {remote}")
        console.print(f"[yellow]Requesting {remote}...[/yellow]")

        resp = self.device.readline().split()
        if resp[0] != 'ack':
            console.print(f"[bold red]{' '.join(resp)}[/bold red]")
            return

        size, expected_checksum = map(int, resp[1:3])

        data = bytearray();
        start = time.time()
        while len(data)<size:
            self.device.send_command('ack')
            chunk = self.device.serial.read(min(CHUNK_SIZE, size-len(data)))
            data.extend(chunk)

            # Show transfer speed
            elapsed = time.time() - start
            speed = len(data) / elapsed / 1024  # in KB/s
            progress = (len(data) / size) * 100
            console.print(f"[yellow]Progress: {progress:.1f}% - {speed:.1f} KB/s[/yellow]")

        checksum = crc32(data)
        if checksum != expected_checksum:
            console.print(f"[bold red]Checksum mismatch! Expected {expected_checksum}, got {checksum}[/bold red]")
        else:
            with open(local,'wb') as f:
                f.write(data)
            console.print(f"[bold green]File {local} downloaded[/bold green]")

    def complete_pull(self, text, line, begidx, endidx):
        parts = line.split()
        if len(parts) <= 2:
            return [f for f in self.remote_files if f.startswith(text)]
        elif len(parts) == 3:
            return glob.glob(text+'*')
        return []

    def do_exit(self, arg):
        "exit                Exit shell"
        console.print("[bold cyan]Goodbye![/bold cyan]")
        return True
    do_quit = do_exit

if __name__ == '__main__':
    if len(sys.argv)<2:
        console.print(f"[bold red]Usage: {sys.argv[0]} <serial_port>[/bold red]")
        sys.exit(1)
    device = DeviceClient(sys.argv[1])
    shell = DeviceShell(device)
    try:
        shell.cmdloop()
    except KeyboardInterrupt:
        print("") # Just so it's on a new line :)
        shell.do_exit('')
        sys.exit(0)

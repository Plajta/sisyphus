#!/usr/bin/env python3
"""
hwtest.py — Interactive hardware test runner for Sisyphus.

Firmware must be built with -DSISYPHUS_HWTEST=ON.
Run: python3 hwtest.py
"""

import sys
import time
from dataclasses import dataclass
from enum import Enum
from typing import Optional

from rich import box
from rich.console import Console
from rich.panel import Panel
from rich.table import Table

import protocol as proto

console = Console()

# ── Keyboard layout ───────────────────────────────────────────────────────────
# TCA8418 key code = row*10 + col  (both 1-based)
# Display as (row-1, col-1)  →  (0,0) … (3,3)
KBD_ROWS = 4
KBD_COLS = 4


def all_expected_keys() -> list[tuple[int, int, int]]:
    """Return [(display_row, display_col, key_code), ...] in row-major order."""
    keys = []
    for r in range(KBD_ROWS):
        for c in range(KBD_COLS):
            # TCA8418 code: row * 10 + col + 1
            keys.append((r, c, r * 10 + c + 1))
    return keys


# ── Result tracking ───────────────────────────────────────────────────────────


class Result(Enum):
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    RERUN = "RERUN"


@dataclass
class TestResult:
    name: str
    result: Result = Result.SKIP
    note: str = ""


results: list[TestResult] = []

# ── Protocol helpers ──────────────────────────────────────────────────────────


def send_ack(device: proto.ProtocolClient, cmd: str, timeout: float = None) -> tuple[bool, str]:
    if timeout is not None:
        device.serial.timeout = timeout
    device.send_command(cmd)
    reply = device.readline()
    if timeout is not None:
        device.serial.timeout = 1  # restore default
    if not reply:
        return False, "timeout"
    return reply.startswith("ack"), reply


def send_ack_data(device: proto.ProtocolClient, cmd: str, timeout: float = None) -> tuple[bool, list[str]]:
    if timeout is not None:
        device.serial.timeout = timeout
    device.send_command(cmd)
    reply = device.readline()
    if timeout is not None:
        device.serial.timeout = 1
    if not reply:
        return False, ["timeout"]
    parts = reply.split()
    if parts and parts[0] == "ack":
        return True, parts[1:]
    return False, parts


def not_available(data: list[str]) -> bool:
    return "not" in data and "available" in data


# ── UI helpers ────────────────────────────────────────────────────────────────


def header(idx: int, total: int, name: str) -> None:
    console.print()
    console.rule(f"[bold cyan][{idx}/{total}] {name}[/bold cyan]")


def ask_pass_fail(prompt: str = "  Passed?") -> Result:
    """Prompt for pass/fail. Enter or 'p' = PASS, 'f' = FAIL, 'r' = RERUN, Ctrl+C = SKIP."""
    try:
        while True:
            ans = console.input(f"{prompt} [bold]\\[P/f/r]:[/bold] ").strip().lower()
            if ans in ("", "p"):
                return Result.PASS
            if ans == "f":
                return Result.FAIL
            if ans == "r":
                return Result.RERUN
            console.print("  [dim]Enter or p = pass, f = fail, r = rerun[/dim]")
    except KeyboardInterrupt:
        console.print("\n  [yellow]Skipped[/yellow]")
        return Result.SKIP


def wait_enter(prompt: str = "  Press [bold]Enter[/bold] to continue...") -> None:
    try:
        console.input(prompt)
    except KeyboardInterrupt:
        raise  # let callers catch it for skip


# ── Tests ─────────────────────────────────────────────────────────────────────

# 1 ── LED ─────────────────────────────────────────────────────────────────────


def test_led(device, idx, total) -> TestResult:
    header(idx, total, "LED Test")
    result = TestResult("LED")

    colors = [
        ("RED", 255, 0, 0),
        ("GREEN", 0, 255, 0),
        ("BLUE", 0, 0, 255),
        ("WHITE", 64, 64, 64),
    ]

    try:
        for name, r, g, b in colors:
            ok, reply = send_ack(device, f"hwtest_led {r} {g} {b}")
            if not ok:
                if "not available" in reply:
                    result.result = Result.SKIP
                    result.note = "LED not compiled in"
                    return result
                console.print(f"  [red]Error: {reply}[/red]")
                result.result = Result.FAIL
                return result
            wait_enter(f"  LED → [bold]{name}[/bold]. Press Enter for next...")
        send_ack(device, "hwtest_led_off")
        console.print("  Did all colours look correct?")
        result.result = ask_pass_fail()
    except KeyboardInterrupt:
        send_ack(device, "hwtest_led_off")
        result.result = Result.SKIP

    return result


# 2 ── Keyboard ────────────────────────────────────────────────────────────────


def test_keyboard(device, idx, total) -> TestResult:
    header(idx, total, "Keyboard Test")
    result = TestResult("Keyboard")

    # Quick availability check
    ok, data = send_ack_data(device, "hwtest_keyboard_poll")
    if not ok and not_available(data):
        result.result = Result.SKIP
        result.note = "Keyboard not compiled in"
        console.print("  [yellow]Keyboard not available.[/yellow]")
        return result

    expected = all_expected_keys()  # [(row0, col0, code), ...]
    total_keys = len(expected)

    console.print("  Follow the prompts — press each key when asked.")
    console.print(f"  {KBD_ROWS} rows × {KBD_COLS} cols = {total_keys} keys total.\n")

    try:
        for display_row, display_col, expected_code in expected:
            console.print(
                f"  → Press key [bold]({display_row}, {display_col})[/bold]"
                f"  [dim](code {expected_code})[/dim]",
                end="  ",
            )

            # Poll the ring buffer until this exact key is pressed
            while True:
                ok, data = send_ack_data(device, "hwtest_keyboard_poll")
                if ok:
                    count = int(data[0]) if data else 0
                    found = False
                    for i in range(count):
                        base = 1 + i * 2
                        if base + 1 < len(data):
                            key_code = int(data[base])
                            state = data[base + 1]
                            if state == "pressed":
                                if key_code == expected_code:
                                    found = True
                                else:
                                    # Wrong key pressed — show it but keep waiting
                                    console.print(
                                        f"\n  [yellow]Got key ({key_code // 10}, "
                                        f"{key_code % 10 - 1}) — "
                                        f"expected ({display_row}, {display_col}). Try again...[/yellow]",
                                        end="  ",
                                    )
                    if found:
                        break
                time.sleep(0.05)

            console.print("[green]✓[/green]")

        console.print(
            f"\n  [green]All {total_keys} keys registered! Auto-pass.[/green]"
        )
        result.result = Result.PASS

    except KeyboardInterrupt:
        console.print("\n  [yellow]Skipped[/yellow]")
        result.result = Result.SKIP

    return result


# 3 ── Audio ───────────────────────────────────────────────────────────────────


def test_audio(device, idx, total) -> TestResult:
    header(idx, total, "Audio Test")
    result = TestResult("Audio")

    try:
        wait_enter("  Press [bold]Enter[/bold] to play audio...")
        ok, reply = send_ack(device, "hwtest_audio", timeout=10.0)
        if not ok:
            console.print(f"  [red]Error: {reply}[/red]")
            result.result = Result.FAIL
            return result
        console.print("  Did you hear the audio?")
        result.result = ask_pass_fail()
    except KeyboardInterrupt:
        result.result = Result.SKIP

    return result


# 4 ── Sensor LED ──────────────────────────────────────────────────────────────


def test_sensor_led(device, idx, total) -> TestResult:
    header(idx, total, "Sensor LED Test")
    result = TestResult("Sensor LED")

    try:
        # Active-low: 0 = ON, 1 = OFF
        ok, reply = send_ack(device, "hwtest_sensor_led 0")
        if not ok:
            if "not available" in reply:
                result.result = Result.SKIP
                result.note = "Sensor LED not compiled in"
                console.print("  [yellow]Not available.[/yellow]")
                return result
            result.result = Result.FAIL
            return result

        console.print("  Aux sensor LED is [bold]ON[/bold].")
        console.print("  Did the sensor LED light up?")
        result.result = ask_pass_fail()
    except KeyboardInterrupt:
        result.result = Result.SKIP
    finally:
        send_ack(device, "hwtest_sensor_led 1")

    return result


# 5 ── Color Sensor ────────────────────────────────────────────────────────────

DELTA_THRESHOLD = 50


def test_color_sensor(device, idx, total) -> TestResult:
    header(idx, total, "Color Sensor Test")
    result = TestResult("Color Sensor")

    def read() -> Optional[dict]:
        ok, data = send_ack_data(device, "hwtest_sensor")
        if not ok or len(data) < 4:
            return None
        return {
            "hue": int(data[0]),
            "sat": int(data[1]),
            "val": int(data[2]),
            "clear": int(data[3]),
        }

    try:
        # Quick check
        probe = read()
        if probe is None:
            _, data = send_ack_data(device, "hwtest_sensor")
            if not_available(data):
                result.result = Result.SKIP
                result.note = "Color sensor not compiled in"
                console.print("  [yellow]Not available.[/yellow]")
                return result
            console.print("  [red]Sensor read failed.[/red]")
            result.result = Result.FAIL
            return result

        wait_enter("  Place [bold]Object 1[/bold] in front of sensor, press Enter...")
        r1 = read()
        if r1:
            console.print(
                f"  Reading 1: H=[cyan]{r1['hue']:5d}[/cyan]  "
                f"S=[cyan]{r1['sat']:3d}[/cyan]  "
                f"V=[cyan]{r1['val']:3d}[/cyan]  "
                f"C=[cyan]{r1['clear']:5d}[/cyan]"
            )

        wait_enter("  Place [bold]Object 2[/bold] (different colour), press Enter...")
        r2 = read()
        if r2:
            console.print(
                f"  Reading 2: H=[magenta]{r2['hue']:5d}[/magenta]  "
                f"S=[magenta]{r2['sat']:3d}[/magenta]  "
                f"V=[magenta]{r2['val']:3d}[/magenta]  "
                f"C=[magenta]{r2['clear']:5d}[/magenta]"
            )

        if r1 and r2:
            dc = abs(r2["clear"] - r1["clear"])
            dh = abs(r2["hue"] - r1["hue"])
            if dc >= DELTA_THRESHOLD or dh >= DELTA_THRESHOLD:
                console.print(
                    f"  [green]Values changed (ΔC={dc}, ΔH={dh}) — "
                    f"sensor responding. Auto-pass.[/green]"
                )
                result.result = Result.PASS
            else:
                console.print(
                    f"  [yellow]Low delta (ΔC={dc}, ΔH={dh}) — "
                    f"try more contrasting objects.[/yellow]"
                )
                console.print("  Mark result manually:")
                result.result = ask_pass_fail()
        else:
            result.result = Result.FAIL

    except KeyboardInterrupt:
        result.result = Result.SKIP

    return result


# 6 ── Battery State ───────────────────────────────────────────────────────────

VBUS_NAMES = {"0": "No input", "1": "USB Host", "3": "Adapter", "7": "Boost"}
CHARGE_NAMES = {
    "0": "Not charging",
    "1": "Pre-charging / Trickle",
    "2": "Fast charging",
    "3": "Charge terminated (full)",
}


def test_battery(device, idx, total) -> TestResult:
    header(idx, total, "Battery State Test")
    result = TestResult("Battery State")

    try:
        ok, data = send_ack_data(device, "hwtest_battery")
        if not ok:
            if not_available(data):
                result.result = Result.SKIP
                result.note = "Charger not compiled in"
                console.print("  [yellow]Not available.[/yellow]")
                return result
            console.print(f"  [red]Error: {' '.join(data)}[/red]")
            result.result = Result.FAIL
            return result

        vbus, charge, power_good, vin_dpm, thermal = (
            data[i] if i < len(data) else "?" for i in range(5)
        )

        console.print(
            f"  [bold]VBUS status[/bold]   | Expected: USB Host/Adapter (1/3) | Actual: [bold]{VBUS_NAMES.get(vbus, vbus)}[/bold] ({vbus})"
        )
        console.print(
            f"  [bold]Charge status[/bold] | Expected: Fast/Terminated (2/3)  | Actual: [bold]{CHARGE_NAMES.get(charge, charge)}[/bold] ({charge})"
        )
        pg_str = "[green]YES[/green]" if power_good == "1" else "[red]NO[/red]"
        console.print(
            f"  [bold]Power good[/bold]    | Expected: YES (1)                | Actual: {pg_str} ({power_good})"
        )
        console.print(
            f"  [bold]VIN DPM mode[/bold]  | Expected: 0                      | Actual: {vin_dpm}"
        )
        console.print(
            f"  [bold]Thermal reg.[/bold]  | Expected: 0                      | Actual: {thermal}"
        )
        console.print(
            "  [dim](Connected via USB → expect USB Host + Fast or Terminated)[/dim]"
        )

        # Auto-pass if power is good and we are charging/full
        if power_good == "1" and charge in ("2", "3") and vbus in ("1", "3"):
            console.print("  [green]Status looks correct. Auto-pass.[/green]")
            result.result = Result.PASS
        else:
            console.print("  [yellow]Status looks unusual — confirm manually:[/yellow]")
            result.result = ask_pass_fail()

    except KeyboardInterrupt:
        result.result = Result.SKIP

    return result


# 7 ── Battery Faults ──────────────────────────────────────────────────────────

CHARGE_FAULT_NAMES = {
    "0": "[green]None[/green]",
    "1": "[red]Input fault[/red]",
    "2": "[red]Temperature fault[/red]",
    "3": "[red]Safety timer timeout[/red]",
}
NTC_NAMES = {"0": "Normal", "2": "Warm", "3": "Cool", "5": "Cold", "6": "Hot"}


def test_battery_faults(device, idx, total) -> TestResult:
    header(idx, total, "Battery Fault Test")
    result = TestResult("Battery Faults")

    try:
        ok, data = send_ack_data(device, "hwtest_battery_faults")
        if not ok:
            if not_available(data):
                result.result = Result.SKIP
                result.note = "Charger not compiled in"
                console.print("  [yellow]Not available.[/yellow]")
                return result
            console.print(f"  [red]Error: {' '.join(data)}[/red]")
            result.result = Result.FAIL
            return result

        charge_fault, bat_ovp, boost_fault, watchdog, ntc = (
            data[i] if i < len(data) else "?" for i in range(5)
        )

        def flag(val, label):
            return f"[red]{label}[/red]" if val == "1" else "[green]No[/green]"

        wd_str = "[yellow]YES[/yellow]" if watchdog == "1" else "[green]No[/green]"
        ntc_str = (
            f"[red]{NTC_NAMES.get(ntc, ntc)}[/red]"
            if ntc in ("6", "5")
            else f"[green]{NTC_NAMES.get(ntc, ntc)}[/green]"
        )

        console.print(
            f"  [bold]Charge fault[/bold]  | Expected: None (0)   | Actual: {CHARGE_FAULT_NAMES.get(charge_fault, charge_fault)} ({charge_fault})"
        )
        console.print(
            f"  [bold]Battery OVP[/bold]   | Expected: No (0)     | Actual: {flag(bat_ovp, 'YES')} ({bat_ovp})"
        )
        console.print(
            f"  [bold]Boost fault[/bold]   | Expected: No (0)     | Actual: {flag(boost_fault, 'YES')} ({boost_fault})"
        )
        console.print(
            f"  [bold]Watchdog exp.[/bold] | Expected: No (0)     | Actual: {wd_str} ({watchdog})"
        )
        console.print(
            f"  [bold]NTC temp[/bold]      | Expected: Normal (0) | Actual: {ntc_str} ({ntc})"
        )

        has_fault = (
            charge_fault != "0" or bat_ovp == "1" or boost_fault == "1" or ntc == "6"
        )
        if not has_fault:
            console.print("  [green]No critical faults. Auto-pass.[/green]")
            result.result = Result.PASS
        else:
            if ntc == "6":
                console.print("  [red]BATTERY IS HOT! Auto-fail.[/red]")
                result.result = Result.FAIL
            else:
                console.print("  [red]FAULTS DETECTED — mark result manually:[/red]")
                result.result = ask_pass_fail()

    except KeyboardInterrupt:
        result.result = Result.SKIP

    return result


# 8 ── Lid State ───────────────────────────────────────────────────────────────


def test_lid(device, idx, total) -> TestResult:
    header(idx, total, "Lid State Test")
    result = TestResult("Lid State")

    def read_lid() -> Optional[str]:
        ok, data = send_ack_data(device, "hwtest_lid")
        return data[0] if ok and data else None

    try:
        initial = read_lid()
        if initial is None:
            _, data = send_ack_data(device, "hwtest_lid")
            if not_available(data):
                result.result = Result.SKIP
                result.note = "Lid detect not compiled in"
                console.print("  [yellow]Not available.[/yellow]")
                return result
            result.result = Result.FAIL
            return result

        console.print(f"  Current lid state: [bold]{initial.upper()}[/bold]")

        want1 = "closed" if initial == "open" else "open"
        console.print(f"  [bold]{want1.upper()}[/bold] the lid (waiting...) ", end="")
        while True:
            state1 = read_lid()
            if state1 == want1:
                console.print("[green]✓ detected[/green]")
                break
            time.sleep(0.1)

        want2 = initial
        console.print(f"  [bold]{want2.upper()}[/bold] it again (waiting...) ", end="")
        while True:
            state2 = read_lid()
            if state2 == want2:
                console.print("[green]✓ detected[/green]")
                break
            time.sleep(0.1)

        console.print("  [green]Both transitions correct. Auto-pass.[/green]")
        result.result = Result.PASS

    except KeyboardInterrupt:
        result.result = Result.SKIP

    return result


# ── Summary ───────────────────────────────────────────────────────────────────


def print_summary(results: list[TestResult]) -> None:
    console.print()
    table = Table(title="Test Summary", box=box.ROUNDED)
    table.add_column("Test", style="bold")
    table.add_column("Result", justify="center")
    table.add_column("Note", style="dim")

    passed = failed = skipped = 0
    for r in results:
        if r.result == Result.PASS:
            badge = "[green]PASS ✓[/green]"
            passed += 1
        elif r.result == Result.FAIL:
            badge = "[red]FAIL ✗[/red]"
            failed += 1
        else:
            badge = "[yellow]SKIP[/yellow]"
            skipped += 1
        table.add_row(r.name, badge, r.note)

    console.print(table)
    total_ran = passed + failed
    if failed == 0 and total_ran > 0:
        console.print(
            Panel(
                f"[bold green]ALL PASSED {passed}/{total_ran}[/bold green]"
                + (f"  ({skipped} skipped)" if skipped else ""),
                border_style="green",
            )
        )
    else:
        console.print(
            Panel(
                f"[bold red]FAILED {failed}/{total_ran}[/bold red]"
                f"  passed: {passed}" + (f"  skipped: {skipped}" if skipped else ""),
                border_style="red",
            )
        )


# ── Entry point ───────────────────────────────────────────────────────────────


def parse_test_indices(arg: str, max_val: int) -> list[int]:
    selected = set()
    for part in arg.split(","):
        if "-" in part:
            start, end = part.split("-")
            selected.update(range(int(start), int(end) + 1))
        else:
            selected.add(int(part))
    return sorted([x for x in selected if 1 <= x <= max_val])


if __name__ == "__main__":
    console.print(
        Panel(
            "[bold magenta]Sisyphus Hardware Test Runner[/bold magenta]\n"
            "[dim]Firmware must be built with -DSISYPHUS_HWTEST=ON[/dim]\n"
            "[dim]Enter or P = pass  |  F = fail  |  R = rerun  |  Ctrl+C = skip test[/dim]",
            border_style="magenta",
        )
    )

    try:
        device = proto.ProtocolClient()
    except ConnectionRefusedError as e:
        console.print(f"[bold red]No device found:[/bold red] {e}")
        sys.exit(1)

    info = device.info()
    console.print(
        f"  Device: [cyan]{info.device_name}[/cyan]  "
        f"Build: [cyan]{info.git_commit_sha}[/cyan]  "
        f"Date: [cyan]{info.build_date}[/cyan]"
    )

    tests = [
        test_led,
        test_keyboard,
        test_audio,
        test_sensor_led,
        test_color_sensor,
        test_battery,
        test_battery_faults,
        test_lid,
    ]

    tests_to_run = tests
    if len(sys.argv) > 1:
        try:
            indices = parse_test_indices(sys.argv[1], len(tests))
            if indices:
                tests_to_run = [tests[i - 1] for i in indices]
        except ValueError:
            console.print("[red]Invalid test selection format. Use e.g. 1-3,5[/red]")
            sys.exit(1)

    total = len(tests_to_run)

    try:
        for i, test_fn in enumerate(tests_to_run, start=1):
            while True:
                r = test_fn(device, i, total)
                if r.result == Result.RERUN:
                    console.print("\n  [cyan]Re-running test...[/cyan]")
                    continue
                results.append(r)
                icon = {
                    "PASS": "[green]✓[/green]",
                    "FAIL": "[red]✗[/red]",
                    "SKIP": "[yellow]~[/yellow]",
                }[r.result.value]
                console.print(f"  {icon} {r.name}: [bold]{r.result.value}[/bold]")
                break
    except KeyboardInterrupt:
        console.print("\n[bold yellow]Test run aborted.[/bold yellow]")

    print_summary(results)

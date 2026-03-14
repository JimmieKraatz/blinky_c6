#!/usr/bin/env python3
"""Simple CLI smoke check over serial for blinky_c6.

Usage:
  python tools/hil/cli_smoke.py --port /dev/ttyACM0
"""

from __future__ import annotations

import argparse
import sys
import time
from typing import List, Tuple

try:
    import serial  # type: ignore
except ImportError as exc:  # pragma: no cover
    print(f"pyserial import failed: {exc}", file=sys.stderr)
    print("Install with: python -m pip install pyserial", file=sys.stderr)
    sys.exit(2)


Check = Tuple[str, str]


def wait_for(ser: serial.Serial, needle: str, timeout_s: float) -> bool:
    deadline = time.monotonic() + timeout_s
    buf = ""
    while time.monotonic() < deadline:
        available = ser.in_waiting
        if available <= 0:
            time.sleep(0.02)
            continue
        data = ser.read(min(available, 256))
        if data:
            chunk = data.decode(errors="ignore")
            buf += chunk
            if needle in buf:
                return True
    return False


def run_checks(ser: serial.Serial, checks: List[Check], timeout_s: float) -> int:
    failures = 0
    total = len(checks)
    for idx, (cmd, expect) in enumerate(checks, start=1):
        print(f"[{idx}/{total}] SEND cmd='{cmd}' expect='{expect}'", flush=True)
        wire = f"{cmd}\n"
        try:
            written = ser.write(wire.encode())
            if written <= 0:
                print(f"[FAIL] cmd='{cmd}' write failed (0 bytes)", flush=True)
                failures += 1
                continue
        except serial.SerialTimeoutException:
            print(f"[FAIL] cmd='{cmd}' write timeout", flush=True)
            failures += 1
            continue
        ok = wait_for(ser, expect, timeout_s)
        status = "PASS" if ok else "FAIL"
        print(f"[{status}] cmd='{cmd}' expect='{expect}'", flush=True)
        if not ok:
            failures += 1
    return failures


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Blinky CLI serial smoke check")
    parser.add_argument("--port", required=True, help="Serial device, e.g. /dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baudrate")
    parser.add_argument("--timeout", type=float, default=4.0, help="Per-command timeout (seconds)")
    parser.add_argument("--boot-settle", type=float, default=2.0, help="Initial settle delay (seconds)")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    checks: List[Check] = [
        ("help", "commands: help, status, run, pause, menu enter, menu next, menu exit"),
        ("status", "status: started="),
        ("menu exit", "command ignored in state=running"),
        ("menu next", "command ignored in state=running"),
        ("menu enter", "cmd dispatch: menu_enter"),
        ("menu enter", "command ignored in state=menu"),
        ("menu next", "cmd dispatch: menu_next"),
        ("run", "command ignored in state=menu"),
        ("pause", "command ignored in state=menu"),
        ("menu exit", "cmd dispatch: menu_exit"),
        ("pause", "cmd dispatch: pause"),
        ("pause", "command ignored in state=paused"),
        ("run", "cmd dispatch: run"),
        ("run", "command ignored in state=running"),
    ]

    print(
        f"opening serial port={args.port} baud={args.baud} timeout={args.timeout}s",
        flush=True,
    )
    with serial.Serial(
        args.port,
        args.baud,
        timeout=0,
        write_timeout=0.5,
        rtscts=False,
        dsrdtr=False,
        xonxoff=False,
    ) as ser:
        print(f"connected; settling for {args.boot_settle:.1f}s", flush=True)
        time.sleep(args.boot_settle)
        ser.reset_input_buffer()
        failures = run_checks(ser, checks, args.timeout)

    if failures:
        print(f"cli smoke failed: {failures} check(s) failed", file=sys.stderr)
        return 1
    print("cli smoke passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

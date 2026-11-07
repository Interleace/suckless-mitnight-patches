#!/usr/bin/env python3
"""Simple ASCII UNIX-socket DMX adapter. Template is explicitly provided by caller."""
import argparse
import socket
import sys

p = argparse.ArgumentParser()
p.add_argument('--socket', required=True)
p.add_argument('--template', required=True, help=r'example: set {ch} {value}\n')
p.add_argument('--start', type=int, default=1)
p.add_argument('--frame-template', help=r'one command per RGB frame, placeholders {r} {g} {b} {start}; use this for atomic DMX updates')
a = p.parse_args()
if not 1 <= a.start <= 510:
    p.error('start must be 1..510 (RGB uses three channels)')
template = a.template.encode('utf-8').decode('unicode_escape')
frame_template = a.frame_template.encode('utf-8').decode('unicode_escape') if a.frame_template else None
try:
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
        sock.connect(a.socket)
        for line in sys.stdin:
            parts = line.split()
            if len(parts) != 3:
                continue
            try:
                rgb = [int(x) for x in parts]
            except ValueError:
                continue
            if any(not 0 <= x <= 255 for x in rgb):
                continue
            if frame_template is not None:
                sock.sendall(frame_template.format(r=rgb[0], g=rgb[1], b=rgb[2], start=a.start).encode())
            else:
                for i, value in enumerate(rgb):
                    sock.sendall(template.format(ch=a.start + i, value=value).encode())
except (OSError, KeyError, ValueError) as e:
    print('rgbdmx:', e, file=sys.stderr)
    sys.exit(1)

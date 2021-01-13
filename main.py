import pyaudio
import os
import struct
import numpy as np
import time

from functools import reduce

import os

# If the provided libs don't cut it for you, 
# download one from here: https://github.com/raysan5/raylib/releases/tag/2.0.0
if "RAYLIB_BIN_PATH" not in os.environ:
    os.environ["RAYLIB_BIN_PATH"] = "./raylib/linux_amd64"

import raylibpy as rl

# constants
TARGET_FPS = 60
FORMAT = pyaudio.paInt16     # audio format (bytes per sample?)
CHANNELS = 1                 # single channel for microphone
RATE = 44100                 # samples per second
CHUNK = int(RATE / TARGET_FPS)       # rate / desiredfps samples per frame
SCREEN_WIDTH = 1600
SCREEN_HEIGHT = 900
HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2

print(f"CHUNK size: {CHUNK}")

rl.init_window(SCREEN_WIDTH, SCREEN_HEIGHT, "hello")
rl.set_target_fps(TARGET_FPS)

# pyaudio class instance
p = pyaudio.PyAudio()

# stream object to get data from microphone
stream = p.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
    input=True,
    frames_per_buffer=CHUNK
)

print('stream started')

def print_line(a, b):
    if b is not None:
        # rl.draw_line_ex(a, b, 1, rl.VIOLET)
        rl._rl.DrawLineEx(a, b, 3, rl.VIOLET)
        # rl.draw_line(a[0], a[1], b[0], b[1], rl.VIOLET)

    return b

while not rl.window_should_close():
    # binary data
    data = stream.read(CHUNK, exception_on_overflow=False)  
    
    # convert data to integers
    data_int = struct.unpack(str(CHUNK) + 'h', data)
    
    points = [rl.Vector2(float((x / len(data_int)) * SCREEN_WIDTH), float(HALF_SCREEN_HEIGHT + (y / 2**15) * HALF_SCREEN_HEIGHT)) for (x, y) in enumerate(data_int)]

    rl.begin_drawing()
    rl.clear_background(rl.BLACK)

    reduce(print_line, points)

    rl.draw_text(f"{rl.get_fps()} fps", 5, 5, 20, rl.VIOLET)

    rl.end_drawing()

rl.close_window()
import pyaudio
import os
import struct
import numpy as np
import time

from functools import reduce

import os
os.environ["RAYLIB_BIN_PATH"] = "/home/pi/code/raylib-2.0.0/release/libs/linux/"

import raylibpy as pyray
#from raylib.pyray import PyRay
#from raylib.colors import *

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

#pyray = PyRay()
pyray.init_window(SCREEN_WIDTH, SCREEN_HEIGHT, "hello")
pyray.set_target_fps(TARGET_FPS)

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
        #pyray.draw_line_ex(a, b, 1, pyray.VIOLET)
        pyray.draw_line(a[0], a[1], b[0], b[1], pyray.VIOLET)

    return b

while not pyray.window_should_close():
    # binary data
    data = stream.read(CHUNK, exception_on_overflow=False)  
    
    # convert data to integers
    data_int = struct.unpack(str(CHUNK) + 'h', data)
    
    points = [((x / len(data_int)) * SCREEN_WIDTH, HALF_SCREEN_HEIGHT + (y / 2**15) * HALF_SCREEN_HEIGHT) for (x, y) in enumerate(data_int)]

    pyray.begin_drawing()
    pyray.clear_background(pyray.BLACK)

    reduce(print_line, points)

    pyray.draw_text(f"{pyray.get_fps()} fps", 5, 5, 20, pyray.VIOLET)

    pyray.end_drawing()

pyray.close_window()
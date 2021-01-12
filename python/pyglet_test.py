import pyaudio
import os
import struct
import pyglet
from pyglet.gl import *
import numpy as np
import time

# constants
DESIRED_FRAME_RATE = 30
FORMAT = pyaudio.paInt16     # audio format (bytes per sample?)
CHANNELS = 1                 # single channel for microphone
RATE = 44100                 # samples per second
CHUNK = int(RATE / DESIRED_FRAME_RATE)       # rate / desiredfps samples per frame
SCREEN_WIDTH = 1600
SCREEN_HEIGHT = 900
HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2

print(f"CHUNK size: {CHUNK}")

# pyaudio class instance
p = pyaudio.PyAudio()

window = pyglet.window.Window(width=SCREEN_WIDTH, height=SCREEN_HEIGHT)
fps_display = pyglet.window.FPSDisplay(window=window)

# stream object to get data from microphone
stream = p.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
    input=True,
    frames_per_buffer=CHUNK
)

print('stream started')

x_coords = [(x / CHUNK) * SCREEN_WIDTH for x in range(CHUNK)]

@window.event
def on_draw():
    window.clear()

    # binary data
    data = stream.read(CHUNK)  
    
    # convert data to integers
    data_int = struct.unpack(str(CHUNK) + 'h', data)
    
    # create np array
    # data_np = np.array(data_int)
    y_coords = [HALF_SCREEN_HEIGHT + (y / 2**15) * HALF_SCREEN_HEIGHT for y in data_int]

    point_tuples = list(zip(x_coords, y_coords))
    points = [coord for point in point_tuples for coord in point]

    # add 'degenerate' points
    points = [x_coords[0], y_coords[0]] + points + [x_coords[-1], y_coords[-1]]

    #print(points)
    pyglet.graphics.draw(len(points) // 2, pyglet.gl.GL_LINE_STRIP, ("v2f", points))

    fps_display.draw()

def update(dt):
    pass

pyglet.clock.schedule_interval(update, 1/DESIRED_FRAME_RATE)

pyglet.app.run()
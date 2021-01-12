import pyaudio
import os
import struct
import pygame
import pygame.freetype
import numpy as np
import time

# constants

FORMAT = pyaudio.paInt16     # audio format (bytes per sample?)
CHANNELS = 1                 # single channel for microphone
RATE = 44100                 # samples per second
CHUNK = int(RATE / 30)       # rate / desiredfps samples per frame
SCREEN_WIDTH = 1600
SCREEN_HEIGHT = 900
HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2

print(f"CHUNK size: {CHUNK}")

pygame.init()
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
font = pygame.freetype.SysFont(pygame.freetype.get_default_font(), 24)

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

# for measuring frame rate
frame_count = 0
start_time = time.time()

quit = False
debug = False
clock = pygame.time.Clock()

while not quit:
    
    # binary data
    data = stream.read(CHUNK)  
    
    # convert data to integers
    data_int = struct.unpack(str(CHUNK) + 'h', data)
    
    # create np array
    # data_np = np.array(data_int)
    points = [((x / len(data_int)) * SCREEN_WIDTH, HALF_SCREEN_HEIGHT + (y / 2**15) * HALF_SCREEN_HEIGHT) for (x, y) in enumerate(data_int)]

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            quit = True
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE or event.unicode == "q":
                quit = True
            if event.unicode == "d":
                debug = not debug
    

    # Draw
    screen.fill((0, 0, 0))
    pygame.draw.lines(screen, (255, 0, 255), False, points)

    if debug:
        font.render_to(screen, (0, 0), f"{clock.get_fps():.0f} FPS", fgcolor=(255, 255, 255))

    pygame.display.flip()
    clock.tick()
    frame_count += 1
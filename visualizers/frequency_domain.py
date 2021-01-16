import numpy as np
import raylibpy as rl

from scipy.fft import rfft, rfftfreq
from scipy.signal import savgol_filter

def generate_sine_wave(freq, sample_rate=735, duration=1):
    x = np.linspace(0, duration, sample_rate * duration, endpoint=False)
    frequencies = x * freq
    # 2pi because np.sin takes radians
    y = np.sin((2 * np.pi) * frequencies)
    return y

frame_buffer_len = 3
num_buckets = 40

class FrequencyDomainVis:


    def __init__(self, config):
        self.name = "Frequency Domain"
        self.config = config
        
        self.frequencies = rfftfreq(config.audio_sample_size*frame_buffer_len, 1 / config.audio_sample_rate)
        self.cut_offs = np.geomspace(20, config.audio_sample_rate // 2, num=num_buckets, dtype=int)
        self.bars = [0] * num_buckets

        self.mean_audio = 0
        self.audio_buffer = []
        self.wave_points = [rl.Vector2(0, 0) for i in range(400)]
        self.wave_thickness = 3
        self.background_color = rl.Color(255, 0, 255, 255)

    def on_recieve_audio_data(self, audio_data):
        half_screen_height = self.config.screen_height // 2
        sample_scalar = 2*half_screen_height / self.config.max_audio_sample_value

        self.audio_buffer += [x / 2**15 for x in audio_data] 
        if len(self.audio_buffer) > self.config.audio_sample_size * frame_buffer_len:
            self.audio_buffer = self.audio_buffer[len(self.audio_buffer) - self.config.audio_sample_size * frame_buffer_len:]

        self.mean_audio = np.mean(self.audio_buffer)
        self.audio_buffer = [x - self.mean_audio for x in self.audio_buffer]
        freq_domain = -np.abs(rfft(self.audio_buffer))[:len(self.wave_points)]
        #freq_domain = savgol_filter(freq_domain, 3, 1)

        # Calculate wave points based on audio data
        for i, sample in enumerate(freq_domain):
            self.wave_points[i].x = (i / len(freq_domain)) * self.config.screen_width
            self.wave_points[i].y = -100 + self.config.screen_height + (sample * 2)


    def on_draw(self, debug_mode):
        rl.clear_background(self.background_color)

        # Wave color should be the inverse of the background color
        wave_color = rl.Color(255 - self.background_color.r, 0, 255 - self.background_color.b, 255)
        for i in range(len(self.wave_points) - 1):
             #rl._rl.DrawLineEx(self.wave_points[i], self.wave_points[i+1], self.wave_thickness, wave_color)
            rl._rl.DrawLineEx(self.wave_points[i], rl.Vector2(self.wave_points[i].x, self.config.screen_height), 2, wave_color)

        rl.draw_circle(1598.259, 449.996, 5, rl.RAYWHITE)

        for i in range(len(self.bars)):
            x = (i / len(self.bars)) * self.config.screen_width
            y = self.config.screen_height - self.bars[i]
            width = self.config.screen_width // len(self.bars)
            height = self.bars[i]
            rl.draw_rectangle(x, y, width, height, rl.RAYWHITE)

            #rl.draw_text(f"{self.bars[i]:.2f}", x, 10 + (20 * i), 20, rl.RAYWHITE)
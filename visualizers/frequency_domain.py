import numpy as np
import raylibpy as rl

from scipy.fft import rfft, rfftfreq
from scipy.signal import savgol_filter

frame_buffer_len = 5
num_buckets = 40

class FrequencyDomainVis:


    def __init__(self, config):
        self.name = "Frequency Domain"
        self.config = config
        
        self.audio_buffer = []
        self.wave_points = [rl.Vector2(0, 0) for i in range(400)]
        self.background_color = rl.Color(0, 0, 0, 255)

    def on_recieve_audio_data(self, audio_data):
        half_screen_height = self.config.screen_height // 2
        sample_scalar = 2*half_screen_height / self.config.max_audio_sample_value

        self.audio_buffer += [x / 2**15 for x in audio_data] 
        if len(self.audio_buffer) > self.config.audio_sample_size * frame_buffer_len:
            self.audio_buffer = self.audio_buffer[len(self.audio_buffer) - self.config.audio_sample_size * frame_buffer_len:]

        mean_audio = np.mean(self.audio_buffer)
        self.audio_buffer = [x - mean_audio for x in self.audio_buffer]
        freq_domain = -np.abs(rfft(self.audio_buffer)[:len(self.wave_points)])
        freq_domain = savgol_filter(freq_domain, 3, 1)

        # Calculate points based on audio data
        for i, sample in enumerate(freq_domain):
            self.wave_points[i].x = (i / len(freq_domain)) * self.config.screen_width
            self.wave_points[i].y = -100 + self.config.screen_height + (sample * 2)


    def on_draw(self, debug_mode):
        rl.clear_background(self.background_color)

        wave_color = rl.GOLD
        for x, y in self.wave_points:
            rl.draw_rectangle(x, y, 3, self.config.screen_height - y, wave_color)
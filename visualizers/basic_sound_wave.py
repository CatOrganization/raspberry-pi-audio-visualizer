import raylibpy as rl
from scipy.signal import savgol_filter

class BasicSoundWaveVis:

    def __init__(self, config):
        self.name = "Basic Sound Wave"
        self.config = config
        
        self.wave_points = [rl.Vector2(0, 0) for i in range(config.audio_sample_size)]
        self.wave_thickness = 1
        self.background_color = rl.Color(255, 0, 255, 255)

    def on_recieve_audio_data(self, audio_data):
        half_screen_height = self.config.screen_height // 2
        sample_scalar = half_screen_height / self.config.max_audio_sample_value

        y_max = 0

        # Calculate wave points based on audio data
        for i, sample in enumerate(audio_data):
            self.wave_points[i].x = (i / self.config.audio_sample_size) * self.config.screen_width
            self.wave_points[i].y = half_screen_height + (sample * sample_scalar)

            y_max = max(sample, y_max)

        # Calculate bg color and wave thickess based on wave magnitude
        magnitude = y_max / self.config.max_audio_sample_value
        self.background_color.r = 255 * magnitude
        self.background_color.b = 255 * magnitude

        self.wave_thickness = max(1, int(30 * magnitude))
        

    def on_draw(self, debug_mode):
        rl.clear_background(self.background_color)

        # Wave color should be the inverse of the background color
        wave_color = rl.Color(255 - self.background_color.r, 0, 255 - self.background_color.b, 255)
        for i in range(self.config.audio_sample_size - 1):
            rl.draw_line_v(self.wave_points[i], self.wave_points[i+1], wave_color)

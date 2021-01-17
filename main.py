import pyaudio
import os
import struct

# If the provided libs don't cut it for you, 
# download one from here: https://github.com/raysan5/raylib/releases/tag/2.0.0
if "RAYLIB_BIN_PATH" not in os.environ:
    os.environ["RAYLIB_BIN_PATH"] = "./raylib/linux_amd64"

import raylibpy as rl
import visualizers

class Config:
    pass

config = Config()

# Graphics constants
config.target_fps = 60
config.screen_width = 1600
config.screen_height = 900

# Audio constants
config.audio_sample_rate = 44100 # hz
config.audio_sample_size = config.audio_sample_rate // config.target_fps
config.max_audio_sample_value = 2**15

# Visualizers
visualizers = [
    visualizers.BasicSoundWaveVis(config), 
    visualizers.FrequencyDomainVis(config), 
    visualizers.FrequencyAndWaveVis(config)
]

print(f"audio_sample_size size: {config.audio_sample_size}")

def main():
    current_vis_index = 0

    # Init Raylib window
    rl.init_window(config.screen_width, config.screen_height, "Audio Visualizer")
    rl.set_target_fps(config.target_fps)

    # Open the audio stream
    pa = pyaudio.PyAudio()
    audio_stream = pa.open(
        format=pyaudio.paInt16, 
        channels=1, 
        rate=config.audio_sample_rate, 
        input=True, 
        frames_per_buffer=config.audio_sample_size
    )

    debug_mode = True

    # Enter the main loop
    while not rl.window_should_close():
        raw_audio = audio_stream.read(config.audio_sample_size, exception_on_overflow=False)  
        unpacked_audio = struct.unpack(str(config.audio_sample_size) + 'h', raw_audio)
        
        visualizers[current_vis_index].on_recieve_audio_data(unpacked_audio)

        rl.begin_drawing()

        visualizers[current_vis_index].on_draw(debug_mode)

        key = rl.get_key_pressed()
        if key == ord('d'):
            debug_mode = not debug_mode
        elif key == ord('n'):
            current_vis_index += 1
            current_vis_index %= len(visualizers)
        elif key == ord('p'):
            current_vis_index -= 1
            current_vis_index %= len(visualizers)

        if debug_mode:
            rl.draw_text(f"{rl.get_fps()} fps", 5, 5, 20, rl.RAYWHITE)
            rl.draw_text(f"pressed: {key}", 5, 25, 20, rl.RAYWHITE)
            
            length = rl.measure_text(visualizers[current_vis_index].name, 20)
            rl.draw_text(visualizers[current_vis_index].name, config.screen_width - length - 5, 5, 20, rl.RAYWHITE)

        rl.end_drawing()

    # Cleanup time
    audio_stream.stop_stream()
    audio_stream.close()
    pa.terminate()
    rl.close_window()

if __name__ == "__main__":
    main()
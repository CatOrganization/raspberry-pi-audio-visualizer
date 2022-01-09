# Raspberry Pi Audio Visualizer 

I'll figure this README out eventually, for now I'm dumping some random things in here so I don't forget:

Building Raylib for RPI:
`make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_21 RAYLIB_LIBTYPE=SHARED`

Needs 
- pyaudio (requires setup)
- raylib-py (requires raylib binaries)
- wave?

Convert WAV file to expected format:
```
sox jazz-guitar.wav -c 1 -b 16 -e signed-integer jazz-guitar-mono-signed-int.wav
```
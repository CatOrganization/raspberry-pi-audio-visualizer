# Raspberry Pi Audio Visualizer - C++ (ish) version

This _will_ be an audio visualizer for raspberry pi, it's currently under development.

If you want to try to run this yourself, it...._should_ work.

The code is currently a hybrid between C and C++. Newer stuff is in C++.

## Dependencies

This program depends on alsa asoundlib for streaming audio input from hardware:

```
sudo apt-get install libasound2-dev
```

It also depends on [raylib](https://www.raylib.com) for graphics. See [here](https://www.github.com/raysan5/raylib/wiki/Working-on-Raspberry-Pi) for instructions on building raylib for raspberry pi.

It also depends on `libsndfile` for playback of local WAV files:
```
sudo apt-get install libsndfile-dev
```

## Building

Once your dependencies are all set up, you can build by running `make` in the root directory

## Running

After building you should see the executable `audio_visualizer` in the root directory.

Run the program and tell it which audio input to use

### Real-time Microphone Audio

```
./audio_visualizer hw:0,0
```

If the first argument does _not_ end in `.wav`, it'll be treated as the CAPTURE hardware device. You can list capture hardware devices available to you by running 
```
arecord -l
```
Based on the output of that command, your device name is going to take the form of `hw:x,y[,z]`, 
where `x` is the card number, `y` is the device number, and `z` is the subdevice number (if present) of the capture device you want to use.
 
### Local WAV File

```
./audio_visualizer ./jazz-guitar-mono-signed-int.wav
```

if the first argument ends in `.wav`, it'll be treated as a local WAV file and played back on a loop.
Currently only mono signed-int formatted WAV files are supported. (You can use a tool like `sox` to convert WAV files).

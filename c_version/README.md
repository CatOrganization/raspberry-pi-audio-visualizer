# Raspberry Pi Audio Visualizer - Archived C version

I've since switched to the python implementation in the root directory, but I'm leaving this all here for posterity:


This _will_ be an audio visualizer for raspberry pi, it's currently under development.

If you want to try to run this yourself, good luck...

## Dependencies

This program depends on alsa asoundlib for streaming audio input from hardware:

```
sudo apt-get install libasound2-dev
```

It also depends on [raylib](https://www.raylib.com) for graphics. See [here](https://www.github.com/raysan5/raylib/wiki/Working-on-Raspberry-Pi) for instructions on building raylib for raspberry pi.

## Building

Once your dependencies are all set up, you can build by running `make` in the root directory

## Running

After building you should see the executable `audio_visualizer` in the root directory.

Run the program and tell it which audio input device to use:

```
./audio_visualizer hw:0,0
```

The first argument is the CAPTURE hardware device. You can list capture hardware devices available to you by running 
```
arecord -l
```
Based on the output of that command, your device name is going to take the form of `hw:x,y[,z]`, 
where `x` is the card number, `y` is the device number, and `z` is the subdevice number (if present) of the capture device you want to use.
 


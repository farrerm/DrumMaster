# DrumMaster
Software Drum Machine.   

Dependencies: Ncurses (UI), Libsndfile (File I/O), PortMidi (midi), PortAudio (sample playback).

Directions: download and install third party libraries.  These libraries are not compatible with PulseAudio.  If you are running PulseAudio for Linux, you will need to turn it off.  

Sample paths are hardcoded at lines 291-301.  You can use the three samples provided, or use your own .wav files.

To compile C program, you need to explicitly link with dependencies.

Example: 

g++ -o drum drumMachin17.c -lncurses -lsndfile -lportaudio -lportmidi

Drum machine is configured for Linux 32 bit machine.

Have fun!

![drum2](https://user-images.githubusercontent.com/23005392/55602767-05d42200-571c-11e9-8bac-65053d6fe242.png)

# DrumMaster
Software Drum Machine.   

Dependencies: Ncurses (UI), Libsndfile (File I/O), PortMidi (midi), PortAudio (sample playback).

Directions: download and install third party libraries ncurses, libsndfile, portmidi, portaudio. 

Mac

brew install ncurses

brew install libsndfile

brew install portmidi

brew install portaudio

Linux

https://www.cyberciti.biz/faq/linux-install-ncurses-library-headers-on-debian-ubuntu-centos-fedora/

https://stackoverflow.com/questions/44910504/trying-to-install-libsndfile-on-ubuntu-16

https://schollz.com/blog/portmidi/

https://medium.com/@niveditha.itengineer/learn-how-to-setup-portaudio-and-pyaudio-in-ubuntu-to-play-with-speech-recognition-8d2fff660e94

Works with Mac and Linux.  Tested on Mac Mojave and Linux Ubuntu.

Samples are provided.  You may provide your own drum samples (or any .wav file).  In that case, please change sample file names at lines 259-269.

Provides ncurses interface directly in terminal.  User may move cursor using arrow keys.  Press enter to make a change to the interface.  You can "perform" by entering a pattern, saving it, entering another pattern, then loading the original pattern.  There is no limit to your creativity.

Saving and loading happen in real time with no break in the action!

To compile C program, you need to explicitly link with dependencies.

Example: 

g++ -o drum drumMachine.c -lncurses -lsndfile -lportaudio -lportmidi



Have fun!

![drum2](https://user-images.githubusercontent.com/23005392/55602767-05d42200-571c-11e9-8bac-65053d6fe242.png)

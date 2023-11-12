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

How to use

Use arrow keys to move the cursor around the beat grid.  Pressing 'Enter' places an X on the grid.  The X can represent 3 different drum sounds depending on what row it is in.

the bpm field takes values from 0-999.  You won't hear much if you set it to 0, however.  To modify bpm from its default of 120, move the cursor over the bpm and press enter.  You can then enter the desired bpm.  Press enter a second time to update it.

Saving your current beat and bpm settings, and then loading them again, works much the same way.  Same with starting and stopping the machine.

Future Work

It would be nice to select the time signature and size of the grid.  

Known issue / bug: the load file prompt works but shows a bit of extra text if one repeatedly presses enter without providing any input.



Have fun!

![drum2](https://user-images.githubusercontent.com/23005392/55602767-05d42200-571c-11e9-8bac-65053d6fe242.png)

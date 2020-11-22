# DrumMaster
Software Drum Machine.   

Dependencies: Ncurses (UI), Libsndfile (File I/O), PortMidi (midi), PortAudio (sample playback).

Directions: download and install third party libraries ncurses, libsndfile, portmidi, portaudio. 

https://www.cyberciti.biz/faq/linux-install-ncurses-library-headers-on-debian-ubuntu-centos-fedora/

https://stackoverflow.com/questions/44910504/trying-to-install-libsndfile-on-ubuntu-16

https://sourceforge.net/p/portmedia/wiki/Installing_portmidi_on_Linux/

https://medium.com/@niveditha.itengineer/learn-how-to-setup-portaudio-and-pyaudio-in-ubuntu-to-play-with-speech-recognition-8d2fff660e94


Drum machine is configured for Linux 32 bit machine.  (Does not work so far on Mac.) Libraries are not compatible with PulseAudio.  If you are running PulseAudio for Linux, you will need to turn it off.  My 32 bit Linux Mint runs PulseAudio.  However, DrumMaster works out of the box on a 64 bit Ubuntu VM.

Samples are provided.  You may provide your own drum samples (or any .wav file).  In that case, please change sample file names at lines 291-301.

To compile C program, you need to explicitly link with dependencies.

Example: 

g++ -o drum drumMachin18.c -lncurses -lsndfile -lportaudio -lportmidi



Have fun!

![drum2](https://user-images.githubusercontent.com/23005392/55602767-05d42200-571c-11e9-8bac-65053d6fe242.png)

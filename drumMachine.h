#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <portmidi.h>
#include <porttime.h>
#include <portaudio.h>
#include <sndfile.h>
#include <ncurses.h>

//declare global variables

float bassGain = .5;
float snareGain = .5;
float hatGain = .5;
float masterGain = .5;

size_t len2;
size_t len4;

int saveFinal = 1;
int loadFinal = 1;

char *BPMtent;
char *BPMtent2;
char *temp1;
char *temp2;

char *fileTemp;

int BPMpointer = 0;
int BPMsize = 3;
int BPMfinal = 1;

int BPM = 125;

int started = 0;

int startx = 0;
int starty = 0;

char ***choices;
int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(int highlightI, int highlightJ);
void writeFile(char *fileName);
void readFile(char *fileName);

#define SAMPLE_RATE       (48000)
#define FRAMES_PER_BUFFER   (8)
#define NUM_SECONDS          (10)
#define DITHER_FLAG           (0)

#define TIME_PROC ((int32_t (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
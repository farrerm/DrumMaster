

#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include <math.h>
#include "portaudio.h"
//#include "pa_linux_alsa.h"
#include "unistd.h"
#include "pthread.h"
#include "sndfile.h"
#include <ncurses.h>

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

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE       (48000)
#define FRAMES_PER_BUFFER   (8)
#define NUM_SECONDS          (10)
/* #define DITHER_FLAG     (paDitherOff)  */
#define DITHER_FLAG           (0)

#define TIME_PROC ((int32_t (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

PaStreamParameters inputParameters, outputParameters;
    PaStream *stream = NULL;
    PaError err;
    float *bassDrum;
    int bassSize;
   
    int bassIndex;
    
    float *snareDrum;
    int snareSize;
   
    int snareIndex;
   
    float *hatDrum;
 
    int hatSize;
    int hatIndex;
    
    float *outBuff;
    
    int currBeat;

typedef struct
{
    int bpm;
    
    int currBeat;
    
    float *bass;
    int bassSize;
   
    float bassGain;
    int bassIndex;
        
    float *snare;
    int snareSize;
    
    float snareGain;
    int snareIndex;
    
    float *hat;
    int hatSize;
   
    float hatGain;
    int hatIndex;
    
    float masterGain;
    
    char ***choices;
    
    float *outBuff;
    PaStream *stream;
}
paData;    

paData myData;

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    paData *data = (paData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
     float currValue = 0;
    
     for (int i = 0; i < FRAMES_PER_BUFFER; i++ ){         
            
            currValue = 0;
            
                if(data->snareIndex != -1){
                    
                    currValue += data->snare[data->snareIndex] * data->snareGain;
                    data->snareIndex++;
                    if (data->snareIndex == data->snareSize){
                    data->snareIndex = -1;
                    }
                }
                if(data->bassIndex != -1){
                    
                    currValue += data->bass[data->bassIndex]*data->bassGain;
                    data->bassIndex++;
                    if (data->bassIndex == data->bassSize){
                    data->bassIndex = -1;
                    }
                    
                }
                if(data->hatIndex != -1){
                    
                    currValue += data->hat[data->hatIndex] * data->hatGain;
                    data->hatIndex++;
                    if (data->hatIndex == data->hatSize){
                    data->hatIndex = -1;
                    }
                    
                }
                
          //}
              
            currValue = currValue * data->masterGain;
            
            if(currValue <= 1 && currValue >= -1){
               out[i] = currValue;
            }
            else if (currValue < -1){   
                out[i] = -1;
            }
            else if (currValue > 1){
                out[i] =1;
            }          
        }
          
    return paContinue;
}//end PortAudio callback

void process_midi(PtTimestamp timestamp, void *userData)
{
    paData *data = (paData*)userData;
     
    if (data->choices[2][data->currBeat][0] == 88){
       
                data->snareIndex = 0;
    }
    if (data->choices[1][data->currBeat][0] == 88){
       
                data->bassIndex = 0;
    }
    
     if (data->choices[3][data->currBeat][0] == 88){
       
                data->hatIndex = 0;
    }
    
    data->currBeat++;
    if (data->currBeat == 67){
        data->currBeat = 3;
    }
   
}//end PortTime callback

int main(int argc, char *argv[])
{
    
    BPMtent = (char*)malloc(4);
    BPMtent2 = (char*)malloc(4);
    BPMtent = "120";
    temp1 = "";
    temp2 = "";
    
    choices = (char***)malloc(7*sizeof(char**));
    for (int i = 0; i < 7; i++){
        
        choices[i] = (char**)malloc(67*sizeof(char*));
    }
    currBeat = 3;
 
    
    
    choices[0][0] = BPMtent;
    choices[1][0] = "bass";
    choices[2][0] = "snare";
    choices[3][0] = "hat";
    choices[4][0] = "master";
    choices[5][0] = "Start";
    choices[6][0] = "Stop";
    
    choices[5][1] = "Save";
    choices[6][1] = "Load";
    
    choices[5][2] = "";
    choices[6][2] = "";
    
    for (int i = 1; i < 5; i++){
        
        choices[i][1] = "-";
        choices[i][2] = "+";
    }
    
    for(int i = 0; i < 7; i++){
        for (int j = 3; j < 67; j++){
        choices[i][j] = " ";
        }
    }
     
    int highlightI = 0;
        int highlightJ = 0;
    int choice = 0;
    int c;
        //int min = Pa_GetMinNumBuffers(FRAMES_PER_BUFFER, SAMPLE_RATE);
        
        //ncurses initialization
    initscr();
    clear();
    noecho();
    cbreak();    /* Line buffering disabled. pass on everything */
    startx = 0;
    starty = 0;
    
    keypad(stdscr, TRUE);
    refresh();
    print_menu(highlightI, highlightJ);
        //nodelay(stdscr, TRUE);
       // halfdelay(1);
    
        //libsndfile initialization
       
    static float sampleBlock [512] ;
    
    //initialize buffer for use with paData struct
    outBuff = (float*)malloc(FRAMES_PER_BUFFER*8);
    SNDFILE      *infile;
    SNDFILE     *infile2;
    SNDFILE     *infile3;
    
    SF_INFO      sfinfo ;
    int          readcount ;
   
    char         *infilename;
    infilename = "Kick9Mono.wav";
    
    SF_INFO    sfinfo2;
    int     readcount2;
    char    *infilename2;
    
    infilename2 = "Snare4Mono.wav";
    
    SF_INFO sfinfo3;
    int readcount3;
    char *infilename3 = "Hat2Mono.wav";
    
    
    //choices[1][0] = "new";
    //print_menu(highlightI, highlightJ);
    
    if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
    {   /* exit */
        return  1 ;
    } ;
    
     if (! (infile2 = sf_open (infilename2, SFM_READ, &sfinfo2)))
    {   /* exit */
        return  1 ;
    } ;
    
     if (! (infile3 = sf_open (infilename3, SFM_READ, &sfinfo3)))
    {   /* exit. */
        return  1 ;
    } ;
    
    
    //need to count number of samples in sample files so we can
    //allocate arrays for samples using malloc.
    
    //count bass samples
    int counter = 0;
    while ((readcount = sf_read_float (infile, sampleBlock, 512)))
    {        
        if(readcount == 512){
            counter++;
        }
        if(readcount < 512){
            break;
        }
    } ;
    
    int bassSize = counter* 512 + readcount;
    
    sf_close(infile);
    bassDrum = (float*)malloc(bassSize*4);
   
    //count snare samples.
    int counter2 = 0;
    
    while ((readcount2 = sf_read_float(infile2, sampleBlock, 512)))
    {        
        if(readcount2 == 512){
            counter2++;
        }
        if(readcount2 < 512){
            break;
        }
    }
    int snareSize = counter2* 512 + readcount2;
    
    sf_close(infile2);
    snareDrum = (float*)malloc(snareSize*4);
    
    //count hat samples
    int counter3 = 0;
   
    while ((readcount3 = sf_read_float(infile3, sampleBlock, 512)))
    {        
        if(readcount3 == 512){
            counter3++;
        }
        if(readcount3 < 512){
            break;
        }
    }
    int hatSize = counter3* 512 + readcount3;
    
    sf_close(infile3);
    hatDrum = (float*)malloc(hatSize*4);
    
    //now copy sample files into arrays
    
    //copy bass sample into array
    if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
    {   /* exit */
        return  1 ;
    } ;
   
  //  for (int i = 0; i < bassSize; i++){
    //    bassDrum[i] =0;
   // }
    
    int index1 = 0;
    while ((readcount = sf_read_float (infile, sampleBlock, 512)))
    {   
        for (int i = 0; i < readcount; i++ ){
            bassDrum[index1 + i] = sampleBlock[i];            
        }
        index1 += readcount;
     
        if (readcount < 512){
            break;
        }
    } ;
   
    //copy snare sample file into bassDrum array
    if (! (infile2 = sf_open (infilename2, SFM_READ, &sfinfo2)))
    {   /* Exit. */
        return  1 ;
    } ;
    
    int index2 = 0;
    readcount2 = 0;
    
    while (readcount2 = sf_read_float(infile2, sampleBlock, 512))
    {   
        for (int i = 0; i < readcount2; i++ ){
            
            snareDrum[index2 + i] = sampleBlock[i];
        }
       
        index2 += readcount2;
      
        if (readcount2 < 512){
            break;
        }
    } ;
    
    //copy hat into hat array
    
    if (! (infile3 = sf_open (infilename3, SFM_READ, &sfinfo3)))
    {   /* Exit. */
        return  1 ;
    } ;
    
    int index3 = 0;
    readcount3 = 0;
   
    while (readcount3 = sf_read_float(infile3, sampleBlock, 512))
    {   
        for (int i = 0; i < readcount3; i++ ){
            
            hatDrum[index3 + i] = sampleBlock[i];
        }
      
        index3 += readcount3;
      
        if (readcount3 < 512){
            break;
        }
    } ;
    
 
    
    stream = NULL;
    
    //prevent portaudio from writing system err messages to console
    //we want console to look nice for user.
    freopen("/dev/null","w",stderr);
    
    //initialize PortAudio
    Pa_Initialize();
    
    const PaDeviceInfo* outputInfo;
   
    int i;
    int numBytes;
    int numChannels;
    //choices[1][0] = "new";    
   
    //test output
    outputParameters.device = Pa_GetDefaultOutputDevice();  
 
    outputInfo = Pa_GetDeviceInfo( outputParameters.device );   

    outputParameters.device = 0;
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(0)->defaultLowOutputLatency;
     //outputParameters.suggestedLatency = 0;
    //PaAlsaStreamInfo *info;
    //PaAlsa_InitializeStreamInfo(info);
    
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
    {   /* exit. */
        return  1 ;
    } ;
    
    //TIME_START;
    int timeCurr = Pt_Time();
    
    snareIndex = -1;
   
    bassIndex = -1;
    
    hatIndex = -1;
    
   // paData myData;
    myData.bass = bassDrum;
    myData.bassSize = bassSize;
   
    myData.bassIndex = bassIndex;
    myData.stream = stream;
    myData.snare = snareDrum;
    myData.snareSize = snareSize;
   
    myData.snareIndex = snareIndex;
    myData.hat = hatDrum;
    
    myData.hatSize = hatSize;
    myData.hatIndex = hatIndex;
    myData.outBuff = outBuff;
    myData.choices = choices;
    myData.currBeat = currBeat;
    myData.bassGain = bassGain;
    myData.snareGain = snareGain;
    myData.hatGain = hatGain;
    
    myData.masterGain = masterGain;
    
    //PaAlsa_EnableRealtimeScheduling(stream, 1);
    //choices[1][0]= "new";    
    
  //  choices[1][0] = "new";
   // print_menu(highlightI, highlightJ);
    
    err = Pa_OpenStream(
              &stream,
              NULL,
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              patestCallback, /* no callback, use blocking API */
              &myData ); /* no callback, so no callback userData */
   // if( err != paNoError ) goto error2;
   // choices[1][0] = "new";
    print_menu(highlightI, highlightJ);
    
  /*   err = Pa_OpenDefaultStream(
              &stream,
              0,
             1,
              paFloat32,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,      /* we won't output out of range samples so don't bother clipping them */
             // patestCallback, /* no callback, use blocking API */
              //&myData );*/
    
        
  //  PaAlsa_SetNumPeriods(2);
   // PaAlsa_SetRetriesBusy(100);
    // PaAlsa_EnableRealtimeScheduling(stream, 1);
    
   
     
     err = Pa_StartStream( stream );
     
    // PaAlsa_EnableRealtimeScheduling(stream, 1);
    
     int pressedKeys[2];
     int keyCount = 0;
     int keyIndex = 0;
     
    while(1){
    //mvprintw(10, 10, "three");
        
        //choices[6][4] = "three";
       // continue;
        //keyIndex = 0;
        //keyCount = 0;
        
             c = getch();
             
             //if ( c == ERR){
              //   continue;
             //}else{
                  
               // pressedKeys[0] = c;
                //keyCount = 1;
             //}      
             //c = getch();
             
             //if (c != ERR){
               //  if (c != pressedKeys[0]){
                //  pressedKeys[1] = c;
                 // keyCount = 2;
                // }
                 // }
        
        /*if (keyCount == 2){
            
            if(highlightI >=1 && highlightI <= 3 && highlightJ >= 3
                    && highlightJ < 67){
                
                if((pressedKeys[0] == KEY_RIGHT || pressedKeys[1] == KEY_RIGHT)
                        && (pressedKeys[0] == 10 || pressedKeys[1] == 10)){
                    
                    choices[highlightI][highlightJ] = "X";
                    ++highlightJ;
                    
                }
                
               // choices[6][4] = "two";
            }
            //choices[6][4] = "two";
            print_menu(highlightI, highlightJ);
            continue;
        }*/
        
       // c = pressedKeys[0];
        switch(c){    
                    case KEY_UP:
            if(highlightJ == 0){
                                if(highlightI == 0)
                    highlightI = 6;
                else
                    --highlightI;
                        }
                        else if (highlightJ == 1){
                            if(highlightI == 1)
                                highlightI = 6;
                            else
                                    --highlightI;
                            
                        }
                        else if (highlightJ == 2){
                            if(highlightI == 1)
                                highlightI = 4;
                            else
                                --highlightI;
                            
                        }
                        else if (highlightJ >=3 && highlightJ <=66){
                            if(highlightI == 1)
                                highlightI = 3;
                            else
                               --highlightI;
                            
                        }
                break;
            case KEY_DOWN:
                            if(highlightJ == 0){
                                if(highlightI == 6){
                                    highlightI =0;
                                }
                                else
                                    highlightI++;
                                
                            }
                            else if (highlightJ ==1){
                                if(highlightI == 6){
                                    highlightI =1;
                                }
                                else
                                    highlightI++;
                                   
                            }
                            else if (highlightJ == 2){
                                if(highlightI == 4){
                                    highlightI = 1;
                                }
                                else
                                    highlightI++;
                                
                            }
                            else if(highlightJ >=3 && highlightJ <= 66){
                                if (highlightI == 3){
                                    highlightI = 1;
                                }
                                else
                                    highlightI++;
                                
                            }
                                    
                break;
                    case KEY_RIGHT:
                        if(highlightI == 0 && highlightJ ==0)
                            highlightJ = 0;
                        else if(highlightI >=1 && highlightI <=3){
                            if(highlightJ == 66){
                                
                                highlightJ = 0;
                            }
                            else
                                highlightJ++;
                        }
                        else if(highlightI == 4)
                            if(highlightJ == 2){
                                highlightJ = 0;
                            }
                            else
                                highlightJ++;
                        else if(highlightI >= 5 && highlightI <=6 ){
                            if(highlightJ ==1){
                                highlightJ = 0;
                            }
                            else
                                highlightJ++;
                            
                        }
                       
                        break;
                    case KEY_LEFT:
                        if(highlightI == 0 && highlightJ == 0){
                            highlightJ = 0;
                        }
                         else if(highlightI >=1 && highlightI <=3){
                            if(highlightJ == 0){
                                
                                highlightJ = 66;
                            }
                            else
                                highlightJ--;
                        }
                        else if(highlightI == 4)
                            if(highlightJ == 0){
                                highlightJ = 2;
                            }
                            else
                                highlightJ--;
                        else if(highlightI >= 5 && highlightI <=6 ){
                            if(highlightJ ==0){
                                highlightJ = 1;
                            }
                            else
                                highlightJ--;
                            
                        }
                        
                        break;
                    case 48:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 48;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                if (BPMsize < 3){   
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 48;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                                         
                      case 49:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 49;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                               if(BPMsize <3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 49;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                               }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                        case 50:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 50;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if(BPMsize <3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 50;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                          case 51:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 51;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 51;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                                         
                        case 52:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 52;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 52;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                                         
                         case 53:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 53;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 53;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                                         
                        case 54:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 54;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 54;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                        case 55:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 55;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 55;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                                         
                        case 56:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 56;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 56;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
                                         
                        case 57:
                        if(highlightI == 0 && highlightJ == 0){
                            // if BPM final
                            if(BPMfinal == 1 ){
                                
                                BPMtent2[0] = 57;
                                BPMsize = 1;
                                BPMfinal = 0;
                                
                            }
                            else if(BPMfinal == 0){
                                
                                if (BPMsize < 3){
                                //strcat(BPMtent2, temp);
                                BPMtent2[BPMsize] = 57;
                                //BPMtent2[BPMsize] = '0';
                                BPMsize++;
                                //mvprintw(3, 12, "hello");
                                }
                            }
                            choices[0][0] = BPMtent2;    
                        }
                        
                        break;
        //enter            
            case 10:
                //choice = 1;
          //save drum file    
             if (highlightI == 5 && highlightJ == 1){
                 saveFinal = 1;
                 temp1 = "";
                 len2 = 0;
                 if(saveFinal == 1){
                     
                     choices[5][2] = "Enter file name: ";
                     print_menu(highlightI, highlightJ);
                     while(1){
                         
                         //saveFinal = 0;
                         char d = getch();
                         if (d != 10 && d >= 48 && d <= 122){
                         
                         size_t len = strlen(choices[5][2]);
                         len2 = strlen(temp1);
                         
                         char *str = (char*)malloc(len + 1 + 1);
                         char *str2 = (char*)malloc(len2 + 1 + 1);
                         
                         strcpy(str, choices[5][2]);
                         strcpy(str2, temp1);
                        
                         str[len] = d;
                         str2[len2] = d;
                         
                         str[len + 1] = '\0';
                         str2[len2 + 1] = '\0';
                         
                         //char d = getch();  
                         choices[5][2] = str;
                         temp1 = str2;
                         
                         //mvprintw(15, 15, temp1);
                         //mvprintw(16, 16, choices[5][2]);
                         print_menu(highlightI, highlightJ);
                         
                        }
                         else if (d == 10){
                             if(len2 == 0){
                                 saveFinal = 0;
                                 choices[5][2] = "error                  ";
                                 
                                 break;
                             }
                         size_t len3 = strlen(temp1);
                         
                         char *str3 = (char*)malloc(len3 + 4 + 1);
                         strcpy(str3, temp1);
                         str3[len3] = '.';
                         str3[len3+1] = 't';
                         str3[len3+2] = 'x';
                         str3[len3+3] = 't';
                                 str3[len3+4] = '\0';
                         temp1 = str3;
                             //mvprintw(17, 15, temp1);
                             writeFile(temp1);
                             break;
                         }
                     }    
                     
                     //saveFinal = 0;
                     
                 }
                 else if (saveFinal == 0){
                     choices[5][2] = "                 ";
                     saveFinal = 1;
                 }
                 
             }               
             //read drum file
             if (highlightI == 6 && highlightJ == 1){
                 loadFinal = 1;
                 temp2 = "";
                 len4 = 0;
                 if(loadFinal == 1){
                     
                     choices[6][2] = "Enter file name: ";
                     print_menu(highlightI, highlightJ);
                     while(1){
                         
                         //saveFinal = 0;
                         char e = getch();
                         if (e != 10 && e >= 48 && e <= 122){
                            // if(e == KEY_UP || e == KEY_DOWN || e == KEY_LEFT
                                //     || e == KEY_RIGHT){
                              //   break;
                            // }
                         size_t len5 = strlen(choices[6][2]);
                         len4 = strlen(temp2);
                         
                         char *str4 = (char*)malloc(len5 + 1 + 1);
                         char *str5 = (char*)malloc(len4 + 1 + 1);
                         
                         strcpy(str4, choices[6][2]);
                         strcpy(str5, temp2);
                        
                         str4[len5] = e;
                         str5[len4] = e;
                         
                         str4[len5 + 1] = '\0';
                         str5[len4 + 1] = '\0';
                         
                         //char d = getch();  
                         choices[6][2] = str4;
                         temp2 = str5;
                         
                         //mvprintw(15, 15, temp1);
                         //mvprintw(16, 16, choices[5][2]);
                         print_menu(highlightI, highlightJ);
                         
                        }
                         else if (e == 10){
                             if(len4 == 0){
                                 loadFinal = 0;
                                 choices[6][2] = "error                  ";
                                 
                                 break;
                             }
                         size_t len6 = strlen(temp2);
                         
                         char *str6 = (char*)malloc(len4 + 4 + 1);
                         strcpy(str6, temp2);
                         str6[len4 + 1] = '.';
                         str6[len4+2] = 't';
                         str6[len4+3] = 'x';
                         str6[len4+4] = 't';
                                 str6[len4+5] = '\0';
                         temp2 = str6;
                             //mvprintw(17, 15, temp1);
                             readFile(temp2);
                             break;
                         }
                         else{
                             break;
                         }
                     }    
                     
                     //saveFinal = 0;
                     
                 }
                 else if (loadFinal == 0){
                     choices[6][2] = "                 ";
                     loadFinal = 1;
                 }
                 
             }               
                          
             
             if (highlightI == 1 && highlightJ == 1){
                                
                                if (myData.bassGain > 0.0){
                                    myData.bassGain = myData.bassGain - .1;
                                    
                                }
                                if (myData.bassGain == .1){
                                    myData.bassGain = 0;
                                }
                                if (myData.bassGain < .1){
                                    myData.bassGain = 0;
                                }
                                
                            }
              if ( highlightI == 1 && highlightJ == 2){
                                
                                if (myData.bassGain < 1.0){
                                    myData.bassGain = myData.bassGain + .1;
                                    
                                }
                                if (myData.bassGain > .9){
                                    myData.bassGain = 1;
                                }
                            }
              if ( highlightI == 2 && highlightJ == 1){
                                
                                if (myData.snareGain > 0.0){
                                    myData.snareGain = myData.snareGain - .1;
                                    
                                }
                                if (myData.snareGain < .1){
                                    myData.snareGain = 0;
                                }
                               
                            }
              if ( highlightI == 2 && highlightJ == 2){
                                
                                if (myData.snareGain < 1.0){
                                    myData.snareGain = myData.snareGain + .1;
                                    
                                }
                               if (myData.snareGain < .1){
                                   myData.snareGain = 0;
                               }
                            }
              if ( highlightI == 3 && highlightJ == 1){
                                
                                if (myData.hatGain > 0.0){
                                    myData.hatGain = myData.hatGain - .1;
                                    
                                }
                                if (myData.hatGain < .1){
                                    myData.hatGain = 0;
                                }
                            }
                if ( highlightI == 3 && highlightJ == 2){
                               
                                if (myData.hatGain < 1){
                                    myData.hatGain = myData.hatGain + .1;
                                    
                                }
                                if (myData.hatGain > .9){
                                    myData.hatGain = 1;
                                }
                            }
             if ( highlightI == 4 && highlightJ == 1){
                                
                                if (myData.masterGain > 0.0){
                                    myData.masterGain = myData.masterGain - .1;
                                    
                                }
                                if (myData.masterGain < .2){
                                    myData.masterGain = 0;
                                }
                            }
                if ( highlightI == 4 && highlightJ == 2){
                                
                                if (myData.masterGain <= 1){
                                    myData.masterGain = myData.masterGain + .1;
                                    
                                }
                               
                            }
             
                            if (choices[highlightI][highlightJ][0] == 32){
                                
                                //mvprintw(0, 0, "match");
                                choices[highlightI][highlightJ] = "X";
                                
                            }
             
                            else if (choices[highlightI][highlightJ][0] == 88){
                                choices[highlightI][highlightJ] = " ";
                            }
                else if (highlightI == 5 && highlightJ == 0){
                 Pt_Start(BPM, &process_midi, &myData);  
                    
                 started = 1;
                 //printf("hello");
                  //  mvprintw(0, 0, "hello");
                }
                else if (highlightI == 6 && highlightJ == 0){
                    Pt_Stop();
                    started = 0;
                }
                else if (highlightI == 0 && highlightJ == 0){
                    
                    //choices[0][0]= BPMtent;
                    //BPMpointer =0;
                    if (BPMfinal == 0){
                        BPMfinal = 1;
                        
                        float temp2 = 0;
                        for(int i = 0; i < BPMsize; i++){
                            temp2 = temp2 +  pow(10,i)*
                                    (BPMtent2[BPMsize - 1 - i] - 48);
                            
                        }
                    
                        temp2 = temp2 /60;
                        temp2 = temp2 /1000;
                        temp2 = 1/temp2;
                        temp2 = temp2 /4;
                        BPM = temp2;
                        if (started == 1){
                            Pt_Stop();
                            Pt_Start(BPM, &process_midi, &myData);
                            
                            
                        }
                        
                        
                    }
                    break;
                }
                            else{
                                
                            }
                break;
            default:
                //mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
                refresh();
                break;
        }
        print_menu(highlightI, highlightJ);
        if(choice != 0)    /* User did a choice come out of the infinite loop */
            break;
    }//end of ncurses while loop    
    //mvprintw(23, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
    clrtoeol();
    refresh();
    endwin();
    
    
}//end main method

void print_menu(int highlightI, int highlightJ){
    
    curs_set(0);
        int x, y, i;    
        x = 2;
        y = 0;
        if(highlightI == 0 && highlightJ == 0){
            
            attron(A_REVERSE);
            mvprintw(y,x, "bpm: ");
            
            for(int i = 0; i < BPMsize; i++){
                mvprintw(y, x + 5 + i, "%d", choices[0][0][i] - 48);
            }
        //mvprintw(y, x+6, "%d", choices[0][0][1] - 48);
        //mvprintw(y, x +7, "%d", choices[0][0][2] - 48);
        attroff(A_REVERSE);
        
        int difference = 3 - BPMsize;
        if (BPMsize < 3){
            
            for (int i = 0; i < difference; i++){
                
                mvprintw(y, x + 5 + BPMsize + i, " ");
            }
        }
        }
        else
        {
            mvprintw(y,x, "bpm: ");
            
            for(int i = 0; i < BPMsize; i++){
                mvprintw(y, x + 5 + i, "%d", choices[0][0][i] - 48);
            }
          mvprintw(y, x + 5 + BPMsize, "  ");
        }   
            
        /*{
            mvprintw(y,x, "bpm: %d", choices[0][0][0] - 48);
        mvprintw(y, x+6, "%d", choices[0][0][1] - 48);
        mvprintw(y, x +7, "%d", choices[0][0][2] - 48);
        }*/
        
    x = 2;
    y = 2;
    //box(menu_win, 0, 0);
    for(i = 1; i < 5; ++i)
    {    if(highlightI == i && highlightJ == 0) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][0]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][0]);
        ++y;
    }
        y++;
        
       
       
        for(i = 5; i < 7; ++i)
    {    if(highlightI == i && highlightJ == 0) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][0]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][0]);
        ++y;
    }
        x = 12;
        y = 7;
        for(i = 5; i < 7; ++i)
    {    if(highlightI == i && highlightJ == 1) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][1]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][1]);
        ++y;
    }
        
        x = 20;
        y = 7;
        
        //file messages
        for(i = 5; i < 6; ++i)
    {    if(highlightI == i && highlightJ == 1 && saveFinal == 1) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][2]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][2]);
        ++y;
    }
        
        x = 20;
        y = 8;
        
        //file messages
        for(i = 6; i < 7; ++i)
    {    if(highlightI == i && highlightJ == 1 && loadFinal == 1) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][2]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][2]);
        ++y;
    }
        
        
        // - signs
        x = 9;
    y = 2;
    //box(menu_win, 0, 0);
    for(i = 1; i < 5; ++i)
    {    if(highlightI == i && highlightJ == 1) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][1]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][1]);
        ++y;
    }
        
        // + signs
        x = 10;
    y = 2;
    //box(menu_win, 0, 0);
    for(i = 1; i < 5; ++i)
    {    if(highlightI == i && highlightJ == 2) /* High light the present choice */
        {    attron(A_REVERSE);
            mvprintw(y, x, "%s", choices[i][2]);
            attroff(A_REVERSE);
        }
        else
            mvprintw(y, x, "%s", choices[i][2]);
        ++y;
    }
        
        
        //print column labels
        x = 4 + 10;
        y = 0;
        for (int j = 0; j < 4; j++){
        //for(i = 0; i < 8; i++){
            mvprintw(y, x, "%c ", 49 + j );
            
            mvprintw(y, x + 4, "*");
            mvprintw(y, x + 8, "*");
            mvprintw(y, x + 12, "*");
               
            
            x = x +16;
        //}
        
        }
        
        //drum notes
        x = 14;
        y = 2;
        for (int i = 1; i < 4; i++){
            x = 14;
        for(int j = 3; j < 67; j++){
            if(highlightI == i && highlightJ == j){
                attron(A_REVERSE);
                mvprintw(y, x, choices[i][j]);
                attroff(A_REVERSE);    
            }
            else
            {
                mvprintw(y, x, choices[i][j]);   
            }
            x++;
        }
        y++;
        }
        if (saveFinal == 0){
            
            
            
        }
        mvprintw(6, 10, choices[6][4]);
        
    refresh();
}

void writeFile(char *fileName){
    
    FILE *fptr;
    //mvprintw(10, 20, fileName);
    //print_menu(6,1);
    char *temp4 = "/home/matt/Drum14/";
    
    size_t length5 = strlen(temp4);
    size_t length6 = strlen(fileName);
    
    //if (length6 == 5){
        
      //  choices[5][5] = "error: enter a filename";
      //  saveFinal = 0;
        //return;
    //}
    
    char *fName = (char*)malloc(length5 + length6 + 1);
    strcpy(fName, temp4);
    
    for(int i = 0; i< length6; i++){
        
        fName[length5 + i] = fileName[i];
    }
    fName[length5+length6] = '\0';
    
    
    //strcat(temp4, fileName);
    
    fptr = fopen(fName, "w");
    
    if (fptr == NULL){
        
        mvprintw(20, 20, "Error!");
        exit(1);
    }
    
    
    fprintf(fptr, choices[0][0]);
    fprintf(fptr, "\n");
    
    int size = BPMsize;
    
    fprintf(fptr, "%d\n", size);
    
    fprintf(fptr, "%d\n", BPM );
    fprintf(fptr, "%f\n", myData.bassGain);
    fprintf(fptr, "%f\n", myData.snareGain);
    fprintf(fptr, "%f\n", myData.hatGain);
    fprintf(fptr, "%f\n", myData.masterGain);
    
    for(int i = 3; i < 67; i++){
        fprintf(fptr, choices[1][i]);
       // fprintf(fptr, "%s", "\n");
        
    }
    //fprintf(fptr, "%c", '\0');
    fprintf(fptr, "\n");
    
    for(int i = 3; i < 67; i++){
        
        fprintf(fptr, choices[2][i]);
    }
    fprintf(fptr, "\n");
    
    for(int i = 3; i < 67; i ++){
        
        fprintf(fptr, choices[3][i]);
    }
    fprintf(fptr, "\n");
    fprintf(fptr, "%d\n", started);
    fclose(fptr);
    saveFinal = 0;
    choices[5][2] = "saved                              ";
   // size_t length5 = strlen(temp4);
    //size_t length6 = strlen(fileName);
    
    //char *temp8 = malloc(length5 + length6 + 1);
    
    //strcpy(temp8, temp4);
    
    
    //FILE *fptr;
    
    //fptr = fopen(fileName, )
    
}

void readFile(char *fileName){
    
    FILE *fptr;
    
    //mvprintw(10, 10, fileName);
    
    char *temp4 = "/home/matt/Drum14/";
    
    size_t length5 = strlen(temp4);
    size_t length6 = strlen(fileName);
    
    if (length6 == 5){
        
        choices[6][2] = "error: enter a filename";
        loadFinal = 0;
        return;
    }
    
    char *fName = (char*)malloc(length5 + length6 + 1);
    strcpy(fName, temp4);
    
    for(int i = 0; i< length6; i++){
        
        fName[length5 + i] = fileName[i];
    }
    
    fName[length5+length6] = '\0';
    
    //strcat(temp4, fileName);
    
    fptr = fopen(fName, "r");
    
    if (fptr == NULL){
        
        choices[6][2] = "Error!                 ";
        print_menu(6,1);
        loadFinal = 0;
        return;
        //exit(1);
    }
    //stop drum machine
    //Pt_Stop();
    
    //read data from file, store in drum machine
    
    char buff[128];
    
    fgets(buff, 6, fptr);
    
    char *bpm2;
 
    //char * temp5;
    //int size7 = 0;
    
    int counter = 0;
    
    for(counter = 0; counter<6; counter++){
        
        //temp5 = malloc(counter + 2);
        char ch = buff[counter];
        if (ch == '\n'){
            break;
        }
        //temp5[counter] = ch;
    }
    bpm2 = (char*)malloc(counter+1);
    
    for(int i = 0; i < counter; i++){
        bpm2[i] = buff[i];
    }
    bpm2[counter] = '\0';
    //sscanf(buff, "%s", bpm2);
   
    choices[0][0]= bpm2;
    
    fgets(buff, 5, fptr);
    
    int bpmSize2;
    
    sscanf(buff, "%d", &bpmSize2);
    
    BPMsize = bpmSize2;
    
    int BPMnext;
    
    fgets(buff, 8, fptr);
    
    sscanf(buff, "%d", &BPMnext);
    
    BPM = BPMnext;
    
    float bassGainNext;
    
    fgets(buff, 10, fptr);
    
    sscanf(buff, "%f", &bassGainNext);
    
    bassGain = bassGainNext;
    myData.bassGain = bassGainNext;
    
    float snareGainNext;
    
    fgets(buff, 10, fptr);
    sscanf(buff, "%f", &snareGainNext);
    
    snareGain = snareGainNext;
    myData.snareGain = snareGainNext;
    
    float hatGainNext;
    
    fgets(buff, 10, fptr);
    sscanf(buff, "%f", &hatGainNext);
    
    hatGain = hatGainNext;
    myData.hatGain = hatGainNext;
    
    float masterGainNext;
    
    fgets(buff, 10, fptr);
    sscanf(buff, "%f", &masterGainNext);
    
    masterGain = masterGainNext;
    myData.masterGain = masterGainNext;
       
    char temp[65];
    
    fgets(temp, 65, fptr);
    
    //mvprintw(10, 10, buff);
    
    //choices[1][3] = buff;
    
    char *temp66 = (char*)malloc(2);
    
    //load bass notes
    int index;
    for (int i = 0; i<64; i++){
        
        if (temp[i] == 88){
            choices[1][3 + i] = "X";
        }
        else if (temp[i] == 32){
            choices[1][3 + i] = " ";
        }
    }
    
    char temp2[65];
    
    fgets(temp2, 65, fptr);
    
    for (int i = 0; i<64; i++){
        
        if (temp2[i] == 88){
            choices[2][3 + i] = "X";
        }
        else if (temp2[i] == 32){
            choices[2][3 + i] = " ";
        }
    }
    
    char temp3[65];
    
    fgets(temp3, 65, fptr);
    
    for (int i = 0; i<64; i++){
        
        if (temp3[i] == 88){
            choices[2][3 + i] = "X";
        }
        else if (temp3[i] == 32){
            choices[2][3 + i] = " ";
        }
    }
    
    char temp5[65];
    
    fgets(temp5, 65, fptr);
    
    char temp6[65];
    
    fgets(temp6, 65, fptr);
    
    for (int i = 0; i<64; i++){
        
        if (temp6[i] == 88){
            choices[3][3 + i] = "X";
        }
        else if (temp6[i] == 32){
            choices[3][3 + i] = " ";
        }
    }
    
    
    /*
    fgets(buff, 64, fptr);
    
    for (int i = 0; i<64; i++){
        
        choices[2][2+i] = buff[i];
    }
    fgets(buff, 64, fptr);
    
    for(int i = 0; i <64; i++){
        
        choices[3][2+i] = buff[i];
    }*/
    
    //BPMfinal = 1;
    //fgets(buff, 3, fptr);
    
    //int BPMsize5;
    
    //sscanf(buff, "%d", &BPMsize5);
    
    //BPMsize5++;
    
    //BPMsize = BPMsize5;
    
   //int size2;
   
   //char *size3;
   
   //fgets(size3, 5, fptr);
   
   //sscanf(size3, "%d", size2);
   
   //BPMsize = size2;
    
    char temp7[65];
    
    fgets(temp7, 65, fptr);
    
    
    int startedNext;
    fgets(buff, 8, fptr);
    
    sscanf(buff, "%d", &startedNext);
    
    started = startedNext;
    
 
    
    //call printmenu
   Pt_Stop();
   if (started == 1){
   
   Pt_Start(BPM, &process_midi, &myData);
   }
   //started = 1;
      
 //   fclose(fptr);
    loadFinal = 0;
    choices[6][2] = "settings loaded                              ";
     print_menu(6, 1);
}




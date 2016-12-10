#include <SPI.h>
#include "lib_Gamebuino.h"
Gamebuino gb;
//----------------------------------------------------------------------    
//                             F X - S Y N T H
//                              by Yoda Zhang
//----------------------------------------------------------------------    

//----------------------------------------------------------------------------    
// define variables and constants
//----------------------------------------------------------------------------    
byte parameter[8];
byte highest[8] = {1, 58, 116, 19, 7, 19, 7, 57};
byte pointer;
byte i;
byte anim;
byte soundno;

byte soundfx[14][8] = {
  // PAQMAN
  {0,30,59,1,7,0,2,5},  // 0  = backgroundsound 1
  {0,35,57,1,7,0,2,5},  // 1  = backgroundsound 2
  {0,6,62,1,0,0,2,5},   // 2  = frightened backgroundsound
  {1,11,66,1,0,0,7,3},  // 3  = eat dots 1
  {1,44,50,1,0,0,7,3},  // 4  = eat dots 2
  {0,0,2,1,0,0,7,5},    // 5  = eat ghost
  {0,0,108,1,0,0,7,5},  // 6  = eat fruit
  {0,54,44,1,0,0,7,50}, // 7  = dead

  // INVADERS
  {1,57,57,1,1,1,5,6},  // 8  = sound shoot
  {0,0,68,1,0,0,7,4},   // 9  = sound invader hit
  {1,15,57,1,1,2,7,15}, // 10 = sound player hit
  {0,10,60,1,0,0,7,6},  // 11 = sound saucer
  {0,5,58,0,1,5,5,2},   // 12 = sound invaders 1
  {0,4,58,0,1,5,5,2},   // 13 = sound invaders 2
//  {0,2,58,0,1,5,5,2}, // 14 = sound invaders 3
//  {0,1,58,0,1,5,5,2}, // 15 = sound invaders 4
};

//----------------------------------------------------------------------------    
// define images
//----------------------------------------------------------------------------    
extern const byte PROGMEM gamelogo[];
extern const byte PROGMEM skin[];
extern const byte PROGMEM waveform[2][7];
//----------------------------------------------------------------------------    
// setup
//----------------------------------------------------------------------------    
void setup() {
  gb.begin();
  gb.titleScreen(F("    Yoda's"),gamelogo);
  gb.battery.show=false;

  for (i=0 ; i<8; i++) {
    parameter[i]=soundfx[0][i];  
  }
/*
  parameter[0]=0;  // osc waveform:        0 = square, 1 = noise
  parameter[1]=30; // osc pitch:           0 to 58 semitones
  parameter[2]=58;  // osc pitch mod depth: 0-116 (-58 to 58 semitones)
  parameter[3]=0;  // osc pitch mod time:  0 to 19
  parameter[4]=7;  // volume mod depth:    0-7 (-7 to 0 steps)
  parameter[5]=0;  // volume mod time:     0 to 19 
  parameter[6]=7;  // sound volume:        1 to 7 
  parameter[7]=10; // gate time:           0 to 57
*/

  pointer=0;
  soundno=0;
}
//----------------------------------------------------------------------------    
// loop
//----------------------------------------------------------------------------    
void loop() {
  if(gb.update()){

     // button up = move to previous slider
    if (gb.buttons.pressed(BTN_UP)) {
      pointer=--pointer % 8;
    }
    
    // button down = move to next slider
    if (gb.buttons.pressed(BTN_DOWN)) {
      pointer=++pointer % 8;
    }
    
    // button left = decrease value
    if (gb.buttons.repeat(BTN_LEFT, 2)) {
      if (pointer<7) {
        parameter[pointer+1]=--parameter[pointer+1];
        if (parameter[pointer+1]>highest[pointer+1]) {
          parameter[pointer+1]=highest[pointer+1];
        }
      } else {
        for (i=0 ; i<8; i++) {
          soundfx[soundno][i]=parameter[i];  
        }
        soundno=--soundno;
        if (soundno > 13) { soundno=13; }                
        for (i=0 ; i<8; i++) {
          parameter[i]=soundfx[soundno][i];  
        }
      }
      if (pointer==5 and parameter[6]==0) { parameter[6]=7; }  
    }

    // button right = increase value
    if (gb.buttons.repeat(BTN_RIGHT, 2)) {
      if (pointer<7) {
        parameter[pointer+1]=++parameter[pointer+1];
        if (parameter[pointer+1]>highest[pointer+1]) {
          parameter[pointer+1]=0;
        }
      } else {
        for (i=0 ; i<8; i++) {
          soundfx[soundno][i]=parameter[i];  
        }
        soundno=++soundno % 14;
        for (i=0 ; i<8; i++) {
          parameter[i]=soundfx[soundno][i];  
        }
      }
    }

    if (parameter[6]==0) { parameter[6]=1; }

    // button B = change waveform
    if (gb.buttons.pressed(BTN_B)) {
      parameter[0]=++parameter[0] % 2;  
    }
    
    // button A = play sound
    if (gb.buttons.pressed(BTN_A)) {
        for (i=0 ; i<8; i++) {
          soundfx[soundno][i]=parameter[i];  
        }
      playsoundfx(soundno,0);
    }

    // show template
    gb.display.drawBitmap(0,0,skin);

    // show cursor
    anim=++anim % 2;
    if (anim==0) { gb.display.fillRect(0,pointer*6,11,5); }

    // show waveform
    gb.display.drawBitmap(0,0,waveform[parameter[0]]);

    // show pitch slider
    gb.display.drawRect(parameter[1]+12,0,3,5);
    // show pitch modulation depth slider
    gb.display.drawRect(40+(parameter[2]-58)/2,6,3,5);
    // show pitch modulation time slider
    gb.display.drawRect((parameter[3]*3)+12,12,3,5);
    // show volume modulation depth slider
    gb.display.drawRect(13+parameter[4]*8,18,3,5);      
    // show volume modulation time slider
    gb.display.drawRect(12+parameter[5]*3,24,3,5);
    // show volume slider
    gb.display.drawRect(5+parameter[6]*9,30,3,5);
    // show sound length slider
    gb.display.drawRect(12+parameter[7],36,3,5);
    // show sound number
    if (anim==0) { gb.display.fillRect(13+soundno*5,42,5,5); }

    // show slider values
    for (i=1 ; i<8 ; i++) {
      gb.display.cursorX=73;
      gb.display.cursorY=i*6-6;
      gb.display.print(parameter[i]);
    }
    gb.display.cursorX=8;
    gb.display.cursorY=0;
    gb.display.print(parameter[0]);

  } // end of update
} // end of loop

 
void playsoundfx(int fxno, int channel) {
  gb.sound.command(0,soundfx[fxno][6],0,channel); // set volume
  gb.sound.command(1,soundfx[fxno][0],0,channel); // set waveform
  gb.sound.command(2,soundfx[fxno][5],-soundfx[fxno][4],channel); // set volume slide
  gb.sound.command(3,soundfx[fxno][3],soundfx[fxno][2]-58,channel); // set pitch slide
  gb.sound.playNote(soundfx[fxno][1],soundfx[fxno][7],channel); // play note
}


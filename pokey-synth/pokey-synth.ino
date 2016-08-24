///////////////////////////////////////////////////////////////////////////////////////////
// 
//       //////   /////  //  // ////// //   // ////// ////  //////
//      //   // //   // // //  //     //   // //   // //  //     
//     //////  //   // ////   /////   ////// //////  //  //  ////
//    //      //   // // //  //          // //      //  //    //
//   //       /////  //  // //////   ////  //     ////   /////
//
//   Digital Synthesizer Based On CO12294 ("POKEY") Sound Chip
//   hotchk155/2015
//
//   VERSION HISTORY
//   1 ddmmyy  initial version
//   
/*
TODO:
  option to set divider range automatically based on last note
  program change
*/
///////////////////////////////////////////////////////////////////////////////////////////
#include "Arduino.h"
#include "EEPROM.h"
#include "Defs.h"
#include "Pokey.h"
#include "LogicalVoice.h"
#include "LogicalChannel.h"
#include "MidiInput.h"
#include "ControlPanel.h"
#include "Storage.h"
#include "PokeySynth.h"
#include "Testing.h"

// The synth data is stored in these global variables. It would be better to encapsulate
// the data in parent objects, but this saves memory by allowing use to reference the 
// objects by 8 bit index rather than pointer
CPokey Pokey1(0);
CPokey Pokey2(1);
CLogicalChannel Channel[MAX_CHANNEL];
CLogicalVoice Voice[MAX_VOICE];
CPokeySynth PokeySynth;
TONE_CONFIG Patch[MAX_CHANNEL];

///////////////////////////////////////////////////////////////////////////////////
void setup()
{  
  PokeySynth.initIO();
  PokeySynth.init();
  //PokeySynth.test();
}

///////////////////////////////////////////////////////////////////////////////////
void loop() 
{
  PokeySynth.run();
}





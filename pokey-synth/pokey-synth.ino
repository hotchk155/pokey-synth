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
CPokeySynth PokeySynth;

///////////////////////////////////////////////////////////////////////////////////
void setup()
{  
  PokeySynth.init();
}

///////////////////////////////////////////////////////////////////////////////////
void loop() 
{
  PokeySynth.run();
}





///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

#include "Arduino.h"
#include "PortIO.h"
#include "EEPROM.h"
#include "Defs.h"
#include "MidiInput.h"
#include "ControlPanel.h"
#include "Storage.h"
#include "Pokey.h"
#include "LogicalVoice.h"
#include "LogicalChannel.h"
#include "PokeySynth.h"


///////////////////////////////////////////////////////////////////////////////////
CPokeySynth::CPokeySynth()
{
  m_flags = 0;
  m_voiceCount = 0;
  
  m_lastTick = 0;
  m_voiceToUpdate = 0;  
  m_channelCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::initPatch(TONE_CONFIG *conf) 
{
  conf->ePokeyMode = CPokey::MODE_8BIT;
  conf->flags = TONE_CONFIG::POKEY_HIHZ|TONE_CONFIG::UNISON;
  conf->transpose = 0;
  conf->fFineTune = 0;
  conf->pitchBendRange = 5;
  conf->portaTime = 0;
  conf->detuneLevel = 0;
  conf->eDetuneMode = TONE_CONFIG::DETUNE_NONE;
  conf->hpf = 0;              // 0-127 high pass filter level
  conf->dist = 0;
  conf->eLFOMode = TONE_CONFIG::LFO_FREE;
  conf->fLFOStep = 0.0625;
  conf->eLFOWave = TONE_CONFIG::WAVE_TRI;
  conf->arpPeriod = 10;
  conf->arpCount = 3;  
  conf->ampEnv.attackSlope = 65535;
  conf->ampEnv.releaseSlope = 65535;
  conf->ampEnv.mode = ENVELOPE::ATT_REL;
  conf->modEnv.attackSlope = 127;
  conf->modEnv.releaseSlope = 127;
  conf->modEnv.mode = ENVELOPE::ATT_REL;  
  conf->modEnv2Pitch = 0;
  conf->modEnvDepth = 0;
  conf->modEnvDest = 0;
  conf->modEnvDestNeg = 0;       
  conf->lfo2Vol = 0;
  conf->lfo2Pitch = 0;
  conf->lfoDepth = 0;
  conf->lfoDest = 0;
  conf->lfoDestNeg = 0;  
  conf->modWheelDest = 0;
  conf->modWheelDestNeg = 0;   
}

///////////////////////////////////////////////////////////////////////////////////
// Configures the synth based on the loaded patches
void CPokeySynth::configure() 
{
  int i;

  // reset synth state
  reset();

  //TODO multi pokey
  
  // one channel (for now)
  m_channelCount = 1;
  
  // configure the POKEY chip based on current patch
  byte voice[8] = {0};  
  m_voiceCount = Pokey1.configure(Patch[0].ePokeyMode, voice);
  //TEST_CONDITION(m_voiceCount==4, 3);  
    
  // assign physical POKEY voices to logical voices
  for(int i=0; i<m_voiceCount; ++i) {
      Voice[i].assign(0, voice[i]);      
  }   
  
  // Assign voices to a channel
  Channel[0].assign(0, 1, &Patch[0]);  
//  Channel[0].assign(0, m_voiceCount, &Patch[0]);  
  Channel[0].start();
}

///////////////////////////////////////////////////////////////////////////////////
// RESET ALL SYNTH STATE
void CPokeySynth::reset() 
{
  int i;
  for(i=0; i<MAX_VOICE; ++i) {
    Voice[i].reset();
  }    
  for(i=0; i<MAX_CHANNEL; ++i) {
    Channel[i].reset();
  }  
}

///////////////////////////////////////////////////////////////////////////////////
// SILENCE ALL SYNTH OUTPUT
void CPokeySynth::quiet() 
{
  int i;
  for(i=0; i<MAX_VOICE; ++i) {
    Voice[i].quiet();
  }    
  for(i=0; i<MAX_CHANNEL; ++i) {
    Channel[i].quiet();
  }
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::initIO() 
{
  pinMode(P_AD0, OUTPUT);
  pinMode(P_AD1, OUTPUT);
  pinMode(P_AD2, OUTPUT);
  pinMode(P_AD3, OUTPUT);

  pinMode(P_DB0, OUTPUT);
  pinMode(P_DB1, OUTPUT);
  pinMode(P_DB2, OUTPUT);
  pinMode(P_DB3, OUTPUT);
  pinMode(P_DB4, OUTPUT);
  pinMode(P_DB5, OUTPUT);
  pinMode(P_DB6, OUTPUT);
  pinMode(P_DB7, OUTPUT);

  pinMode(P_CS0, OUTPUT);
  pinMode(P_CS1, OUTPUT);
  pinMode(P_RW, OUTPUT);

  pinMode(P_LED1, OUTPUT);
  pinMode(P_LED2, OUTPUT);
  pinMode(P_BUTTON, INPUT_PULLUP);

  digitalWrite(P_RW, LOW);
  digitalWrite(P_CS1, HIGH);
  digitalWrite(P_CS0, HIGH);  
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::init()
{


  m_controlPanel.led1(1);
  m_controlPanel.led2(1);  
/*
  if(!m_storage.isInitialised()) {
    defaultToneConfig(&m_conf[0]);
    for(int i=0; i<m_storage.getNumPatches(); ++i) {
      m_storage.savePatch(i, &m_conf[0]);
    }
    m_numPOKEYs = 1;
    m_storage.setNumPokeys(1);
    m_storage.setCurrentPatch(0);
    m_storage.setInitialised();
    m_controlPanel.flashCode(5);    
  }
  else {
    byte patch = m_storage.getCurrentPatch();
    m_storage.loadPatch(patch, &m_conf[0]);
    m_numPOKEYs = m_storage.getNumPokeys();    
  }
*/  
  initPatch(&Patch[0]);//TODO REMOVE
  
  configure();
  delay(100);
  m_midiInput.init();
  /*
  if(m_numPOKEYs > 1) {
    m_controlPanel.led1(0);
    m_controlPanel.led2(0);
    delay(200);
    m_controlPanel.led1(1);
    m_controlPanel.led2(1);
    delay(200);
  }
  */
  m_controlPanel.led1(0);
  m_controlPanel.led2(0);
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::run() 
{
  int i;
  
  // get the system millisecond counter
  unsigned long ms = millis();

   digitalWrite(P_LED1, !(ms&0xFF));

  // have we moved on a millisecond?
  if(m_lastTick != (byte)ms) {    
    m_lastTick = (byte)ms;
    
    // update the channels (run LFO, envelope etc)
    for(i=0; i<m_channelCount; ++i) {
      Channel[i].update((byte)ms);
    }    
  }
  else 
  {
    // maintain the voices
    Voice[m_voiceToUpdate].update();
    if(++m_voiceToUpdate >= m_voiceCount) {
      m_voiceToUpdate = 0;
    }    
  }
  
  // poll for MIDI input
  byte midi = m_midiInput.read();
  if(midi)
  {
    switch(midi & 0xF0) {
        case 0x80: //  Note-off  2  key  velocity  
        case 0x90: //  Note-on  2  key  veolcity  
        case 0xB0: //  Continuous controller  2  controller #  controller value  
        case 0xE0: //  Pitch bend  2  lsb (7 bits)  msb (7 bit      
          // dispatch MIDI message to channels
          for(i = 0; i<m_channelCount; ++i) {            
            if((midi & 0x0F)==Channel[i].m_midiChannel) {                
              Channel[i].handle(midi, m_midiInput.m_params);
            }
          }        
          break;
    }
  }
}

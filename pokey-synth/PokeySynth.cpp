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
//  m_ticks = 0;
  m_voiceToUpdate = 0;  
  m_channelCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::test() 
{
  m_pokey[0].test();
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::defaultToneConfig(TONE_CONFIG *conf) 
{
  conf->ePokeyMode = CPokey::PCFG_8;
  conf->flags = TONE_CONFIG::POKEY_HIHZ;
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
  conf->ampEnv.fAttackStep = 1.0;
  conf->ampEnv.fReleaseStep = 1.0;
  conf->ampEnv.mode = ENVELOPE::ATT_REL;
  conf->modEnv.fAttackStep = 1.0;
  conf->modEnv.fReleaseStep = 1.0;
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
  
  m_pokey[0].which(0);
  m_pokey[1].which(1);

  // configure the POKEY chip based on current patch
  CPokeyChannel *physical_channels[MAX_VOICE] = {0};      
  m_voiceCount = m_pokey[0].configure(m_conf[0].ePokeyMode, !(m_conf[0].flags & TONE_CONFIG::POKEY_HIHZ), &physical_channels[0]);
  
  // assign physical POKEY voices to logical voices
  for(int i=0; i<m_voiceCount; ++i) {
      m_voice[i].assign(&m_chan[0], physical_channels[i]);      
  }  
}

///////////////////////////////////////////////////////////////////////////////////
// RESET ALL SYNTH STATE
void CPokeySynth::reset() 
{
  int i;
  for(i=0; i<MAX_POKEY; ++i) {
    m_pokey[i].reset();
  }  
  for(i=0; i<MAX_VOICE; ++i) {
    m_voice[i].reset();
  }    
  for(i=0; i<MAX_CHANNEL; ++i) {
    m_chan[i].reset();
  }  
}

///////////////////////////////////////////////////////////////////////////////////
// SILENCE ALL SYNTH OUTPUT
void CPokeySynth::quiet() 
{
  int i;
  for(i=0; i<MAX_POKEY; ++i) {
    m_pokey[i].quiet();
  }  
  for(i=0; i<MAX_VOICE; ++i) {
    m_voice[i].quiet();
  }    
  for(i=0; i<MAX_CHANNEL; ++i) {
    m_chan[i].quiet();
  }
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::init()
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
  defaultToneConfig(&m_conf[0]);//TODO REMOVE
  m_conf[1].ePokeyMode = CPokey::PCFG_NONE;
  
  configure();
  delay(100);
  /*
  m_midiInput.init();
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

  // have we moved on a millisecond?
  if(m_lastTick != (byte)ms) {    
    m_lastTick = (byte)ms;
    
    // update the channels (run LFO, envelope etc)
    for(int i=0; i<m_channelCount; ++i) {
      m_chan[i].update((byte)ms);
    }    
  }
  else 
  {
    // maintain the voices
    m_voice[m_voiceToUpdate].update();
    if(++m_voiceToUpdate >= m_voiceCount) {
      m_voiceToUpdate = 0;
    }    
  }
  
  // poll for MIDI input
  byte midi = m_midiInput.read();
  if(midi)
  {
    // dispatch MIDI message to channels
    for(i = 0; i<m_channelCount; ++i) {      
      if((midi & 0x0F) == m_chan[i].m_midiChannel) {  
        m_chan[i].handle(midi, m_midiInput.m_params);
      }
    }        
  }
}




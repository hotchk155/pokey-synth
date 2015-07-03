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
CPokeySynth::CPokeySynth() : m_pokey1(0), m_pokey2(1)
{
  m_dualPatch = 0;
  m_voiceCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::defaultToneConfig(TONE_CONFIG *conf) 
{
  conf->ePokeyMode = CPokey::PCFG_8;
  conf->flags = 0;
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
  conf->lfoDepth = 0;
  conf->arpPeriod = 10;
  conf->arpCount = 3;  
  conf->ampEnv.fAttackStep = 1.0;
  conf->ampEnv.fReleaseStep = 1.0;
  conf->ampEnv.mode = ENVELOPE::ATT_REL;
  conf->modEnv.fAttackStep = 1.0;
  conf->modEnv.fReleaseStep = 1.0;
  conf->modEnv.mode = ENVELOPE::ATT_REL;  
  conf->lfo2Pitch = 0;
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

  // reset the voices
  for(i=0; i<8; ++i) {
    m_voice[i].reset();
  }  
  // reset the channels
  m_chan[0].reset();
  m_chan[1].reset();


  int voices1 = 0;
  int voices2 = 0;
  CPokeyChannel *physical_channels[8] = {0};
  m_dualPatch = 0;
    
  // Configure POKEY1
  voices1 = m_pokey1.configure(m_conf[0].ePokeyMode, !(m_conf[0].flags & TONE_CONFIG::POKEY_HIHZ), &physical_channels[0]);
  if(m_conf[0].flags & TONE_CONFIG::POKEY_DUAL) {
    // first patch requires both chips
    voices1 += m_pokey2.configure(m_conf[0].ePokeyMode, !(m_conf[0].flags & TONE_CONFIG::POKEY_HIHZ), &physical_channels[voices1]);
    m_chan[0].assign(&m_voice[0], voices1, &m_conf[0]);
  }
  else {
    // can have two patches loaded
    voices2 = m_pokey2.configure(m_conf[1].ePokeyMode, !(m_conf[1].flags & TONE_CONFIG::POKEY_HIHZ), &physical_channels[voices1]);
    m_chan[0].assign(&m_voice[0], voices1, &m_conf[0]);
    if(voices2) {
      m_chan[1].assign(&m_voice[voices1], voices2, &m_conf[1]);
      m_dualPatch = 1;
    }
  }
  m_voiceCount = 0;  
  for(int i=0; i<voices1; ++i) {
      m_voice[m_voiceCount++].assign(&m_chan[0], physical_channels[i]);      
  }  
  for(int i=voices1; i<voices1+voices2; ++i) {
      m_voice[m_voiceCount++].assign(&m_chan[1], physical_channels[i]);      
  }    
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::quiet() 
{
  m_pokey1.quiet();
  m_pokey2.quiet();
  for(int i=0; i<8; ++i) {
    m_voice[i].reset();
  }
  m_chan[0].quiet();
  m_chan[1].quiet();
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

  if(!m_storage.isInitialised()) {
    defaultToneConfig(&m_conf[0]);
    for(int i=0; i<m_storage.getNumPatches(); ++i) {
      m_storage.savePatch(i, &m_conf[0]);
    }
    m_storage.setCurrentPatch(0);
    m_storage.setInitialised();
    m_controlPanel.flashCode(5);    
  }
  else {
    byte patch = m_storage.getCurrentPatch();
    m_storage.loadPatch(patch, &m_conf[0]);
  }
  m_conf[1].ePokeyMode = CPokey::PCFG_NONE;
  
  configure();
  delay(100);
  m_midiInput.init();
  m_controlPanel.led1(0);
  m_controlPanel.led2(0);
}

///////////////////////////////////////////////////////////////////////////////////
byte lastTick = 0;
byte ticks = 0;
byte voiceToUpdate = 0;
byte wasHeld = 0;
void CPokeySynth::run() 
{
//  m_pokey1.m_chan[0].test();
//  return;
  int i;
  unsigned long ms = millis();

  byte buttonHold = m_controlPanel.m_buttonHold;
  if(lastTick != (byte)ms) {    
    m_controlPanel.run();
    switch(buttonHold) {
      case CControlPanel::HOLD:
        m_controlPanel.led1(1);      
        wasHeld = 1;
        break;
      case CControlPanel::LONGHOLD:
        m_controlPanel.led1(!!(ms & 0x40));
        wasHeld = 2;
        break;
      default:
        if(wasHeld) {
          quiet();
          if(wasHeld == 2) {
            configure();
          }
          m_controlPanel.led1(0);
          wasHeld = 0;
        }    
        break;
    }
    m_voice[voiceToUpdate].update();
    if(++voiceToUpdate >= m_voiceCount) {
      voiceToUpdate = 0;
    }
    m_chan[0].run(ticks);
    if(m_dualPatch) m_chan[1].run(ticks);
    ++ticks;
    lastTick = (byte)ms;
  }
  
  byte midi = m_midiInput.read();
  if(midi)
  {
    if((midi & 0x0F) == 0) {
      m_chan[0].handle(midi, m_midiInput.m_params);
      m_controlPanel.pulse();    
    }
    else if(((midi & 0x0F) == 1) && m_dualPatch)
      m_chan[1].handle(midi, m_midiInput.m_params);
      m_controlPanel.pulse();    
    }
    
    if(m_chan[0].m_flags & CLogicalChannel::SF_RECONFIG) {
      configure();
      m_chan[0].m_flags &= ~CLogicalChannel::SF_RECONFIG;
    }
        
    if(buttonHold && ((midi & 0xF0) == 0x90) && m_midiInput.m_params[1]) {
      char patch = m_midiInput.m_params[0] % 12;
      if(patch >= 0 && patch < m_storage.getNumPatches()) {
        switch(buttonHold) {
          case CControlPanel::HOLD:
            m_controlPanel.led1(1);
            m_controlPanel.led2(1);
            m_storage.loadPatch(patch, &m_conf[0]);
            m_storage.setCurrentPatch(patch);
            delay(200);
            configure();
            m_controlPanel.led1(0);
            m_controlPanel.led2(0);
            wasHeld = 0;            
            break;
          case CControlPanel::LONGHOLD:
            m_controlPanel.led1(1);
            m_controlPanel.led2(1);
            m_storage.savePatch(patch, &m_conf[0]);
            m_storage.setCurrentPatch(patch);
            delay(500);
            m_controlPanel.led1(0);
            m_controlPanel.led2(0);
            wasHeld = 0;            
            break;
        }
      }
    }
}




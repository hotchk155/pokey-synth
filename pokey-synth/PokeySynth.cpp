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
void CPokeySynth::defaultChannelConfig(CHANNEL_CONFIG *conf) 
{
  conf->ePokey1Mode = CPokey::PCFG_8;
  conf->ePokey2Mode = CPokey::PCFG_NONE;
  
  //lc->req_channel[0] = -1;
  conf->flags = CF_FULLVELOCITY; //|CF_OMNINOTES|CF_OMNICC;  
//  lc->midiChannel = ch;
//  lc->trigMin = 0;
//  lc->trigMax = 127;
  conf->transpose = 0;
  conf->fFineTune = 0.0; 
  conf->pitchBendRange = 2;
  conf->portaTime = 0;
  conf->detuneLevel = 0;
  conf->eDetuneMode = 0;
  conf->hpf = 0;
  conf->dist = 127;
  conf->eLFOMode = RUN_FREE;
  conf->fLFOStep = 0.0625;
  conf->fLFOLevel = 1.0;
  conf->eLFOWave = LFO_TRI;
  conf->fAttackStep = 1.0;
  conf->fReleaseStep = 1.0;
  conf->arpPeriod = 20;
  conf->arpCount = 8;  
  conf->eModWheelDest = MOD2DIST;
  conf->eEnvelopeDest = ENV2VOL;
  conf->eLFODest = LFO2NONE;
}

///////////////////////////////////////////////////////////////////////////////////
/*
void CPokeySynth::defaultGlobalConfig(GLOBAL_CONFIG *cfg) 
{
  cfg->pokey1Mode = CPokey::PCFG_8;
  cfg->pokey2Mode = CPokey::PCFG_8;
  for(int i=0; i<NUM_LOGICAL_CHANNELS; ++i) {
    defaultChannelConfig(i, &cfg->channel_config[i]);
  }
  cfg->channel_config[0].req_channel[0] = 0;
  cfg->channel_config[0].req_channel[1] = 1;
  cfg->channel_config[0].req_channel[2] = 2;
  cfg->channel_config[0].req_channel[3] = 3; 
  cfg->channel_config[0].req_channel[4] = -1; 
  cfg->channel_config[0].req_channel[5] = 5; 
  cfg->channel_config[0].req_channel[6] = 6; 
  cfg->channel_config[0].req_channel[7] = 7; 
//  cfg->channel_config[0].req_channel[4] = -1; 
}
*/

/*
///////////////////////////////////////////////////////////////////////////////////
// Configures the entire POKEY synth based on the content of the GlobalConfig
void CPokeySynth::configure() 
{
  int i;

  // reset all the logical voices
  for(i=0; i<8; ++i) {
    m_logicalVoices[i].reset();
  }
  // reset all the logical channels
  for(i=0; i<NUM_LOGICAL_CHANNELS; ++i) {
    m_logicalChannels[i].reset();
  }

  // Configure POKEY devices and receive pointers to the active channels
  // for each device, which are placed sequentially from index 0 for 
  // device #1 and fom index 4 from devices #2
  CPokeyChannel *physical_channels[8] = {0};
  m_pokey1.configure(m_globalConfig.pokey1Mode, &physical_channels[0]);
  m_pokey2.configure(m_globalConfig.pokey2Mode, &physical_channels[4]);
  
  // Configure the logical channels and voices
  char voice_count = 0;
  for(i=0; i<NUM_LOGICAL_CHANNELS; ++i) {
    CHANNEL_CONFIG *conf = &m_globalConfig.channel_config[i];    
    char req_channel_index = 0;
    char first_voice_for_channel = voice_count;
    char num_voices_for_channel = 0;
    while(req_channel_index < 8 && voice_count < 8)
    {
      char req_channel = conf->req_channel[req_channel_index++];
      if(req_channel < 0) {
        break; // -1 used to indicate end of list of required channels
      }
      if(req_channel < 8 && physical_channels[req_channel] != NULL) {
        // assign the logical channel, physical channel to the logical voice 
        m_logicalVoices[voice_count++].assign(&m_logicalChannels[i], physical_channels[req_channel]);
        num_voices_for_channel++;
      }
    }       
    // assign the block of logical voices to the logical channel
    m_logicalChannels[i].assign(&m_logicalVoices[first_voice_for_channel], num_voices_for_channel, conf);
  }
}
*/

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
  voices1 = m_pokey1.configure(m_conf[0].ePokey1Mode, &physical_channels[0]);
  if(m_conf[0].ePokey2Mode != CPokey::PCFG_NONE) {
    // first patch requires both chips
    voices1 += m_pokey2.configure(m_conf[0].ePokey1Mode, &physical_channels[voices1]);
    m_chan[0].assign(&m_voice[0], voices1, &m_conf[0]);
  }
  else {
    // can have two patches loaded
    voices2 = m_pokey2.configure(m_conf[1].ePokey1Mode, &physical_channels[voices1]);
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
  
  m_controlPanel.flashCode(m_voiceCount);
//  m_controlPanel.led2(1);
}

///////////////////////////////////////////////////////////////////////////////////
void CPokeySynth::quiet() 
{
  m_pokey1.quiet();
  m_pokey2.quiet();
  for(int i=0; i<8; ++i) {
    m_voice[i].quiet();
  }
  for(int i=0; i<NUM_LOGICAL_CHANNELS; ++i) {
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


//  m_controlPanel.led1(1);
//  m_controlPanel.led2(1);  
//  delay(500);

/*
  if(!m_storage.isInitialised()) {
    defaultGlobalConfig(&m_globalConfig); 
    for(int i=0; i<m_storage.getNumPatches(); ++i) {
      m_storage.savePatch(i, &m_globalConfig);
    }
    m_storage.setCurrentPatch(0);
    m_storage.setInitialised();
    m_controlPanel.flashCode(5);    
  }
  else {
    byte patch = m_storage.getCurrentPatch();
    m_storage.loadPatch(patch, &m_globalConfig);
  }*/
  defaultChannelConfig(&m_conf[0]);
  m_conf[1].ePokey1Mode = CPokey::PCFG_NONE;
  m_conf[1].ePokey2Mode = CPokey::PCFG_NONE;
  
  configure();
  delay(500);
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
  //m_pokey1.m_chan[0].test();
//  return;
  int i;
  unsigned long ms = millis();

  byte buttonHold = m_controlPanel.m_buttonHold;
  if(lastTick != (byte)ms) {    
    m_controlPanel.run();
    /*
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
    */
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
    
    /*
    if(buttonHold && ((midi & 0xF0) == 0x90) && m_midiInput.m_params[1]) {
      char patch = m_midiInput.m_params[0] % 12;
      if(patch >= 0 && patch < m_storage.getNumPatches()) {
        switch(buttonHold) {
          case CControlPanel::HOLD:
            m_controlPanel.led1(1);
            m_controlPanel.led2(1);
            m_storage.loadPatch(patch, &m_globalConfig);
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
            m_storage.savePatch(patch, &m_globalConfig);
            m_storage.setCurrentPatch(patch);
            delay(500);
            m_controlPanel.led1(0);
            m_controlPanel.led2(0);
            wasHeld = 0;            
            break;
        }
      }
    }
    else {      
      m_controlPanel.pulse();    
      for(i=0; i<NUM_LOGICAL_CHANNELS; ++i) {
        m_logicalChannels[i].handle(midi, m_midiInput.m_params);
      }
    }
    */
}




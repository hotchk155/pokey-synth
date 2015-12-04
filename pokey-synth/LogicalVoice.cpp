///////////////////////////////////////////////////////////
//
// LOGICAL VOICE
//
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

#include "Arduino.h"
#include "Defs.h"
#include "Pokey.h"
#include "LogicalVoice.h"
#include "LogicalChannel.h"

///////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
CLogicalVoice::CLogicalVoice()
{
  m_channel = 0;
  m_voice = 0;
  reset();
}

///////////////////////////////////////////////////////////////////////
// ASSIGN POKEY CHANNEL AND LOGICAL CHANNEL TO VOICE
void CLogicalVoice::assign(byte channel, byte voice)
{
  m_channel = channel;  
  m_voice = voice;
}

///////////////////////////////////////////////////////////////////////
// RESET THE VOICE STATE
void CLogicalVoice::reset()
{
  m_midi_note = 0;
  m_midi_vel = 0;
  m_amp.ePhase = m_mod.ePhase = ENVELOPE_STATE::NONE;
  m_amp.fValue = m_mod.fValue = 0.0;
  m_mod.ePhase = m_mod.ePhase = ENVELOPE_STATE::NONE;
  m_mod.fValue = m_mod.fValue = 0.0;
}

///////////////////////////////////////////////////////////////////////
// SILENCE THE VOICE
void CLogicalVoice::quiet()
{
  CPokey *pokey = (m_voice & VOICE_POKEY2) ? &Pokey2 : &Pokey1;
  pokey->vol(m_voice, 0);
}

///////////////////////////////////////////////////////////////////////
// SET THE DIVIDER RANGE
void CLogicalVoice::div8_high(byte v) {
  CPokey *pokey = (m_voice & VOICE_POKEY2) ? &Pokey2 : &Pokey1;
  pokey->div8_high(m_voice,v);
}

///////////////////////////////////////////////////////////////////////
// SET THE DISTORTION POLYNOMIAL TYPE
void CLogicalVoice::dist_poly9(byte v) {
  CPokey *pokey = (m_voice & VOICE_POKEY2) ? &Pokey2 : &Pokey1;
  pokey->dist_poly9(m_voice,v);
}

///////////////////////////////////////////////////////////////////////
// UPDATE LOGICAL VOICE
// This is the all important point where the triggers, modulation etc
// get applied to the assigned POKEY voice
void CLogicalVoice::update() 
{    
  CPokey *pokey = (m_voice & VOICE_POKEY2) ? &Pokey2 : &Pokey1;

  float value;
  
  CLogicalChannel *channel = &Channel[m_channel];
  TONE_CONFIG *conf = channel->m_conf;
  
  ////////////////////////////////////////////////////////////////////////////////
  // PITCH CALCULATION
  ////////////////////////////////////////////////////////////////////////////////
  
  // get initial value
  if(channel->m_portaTargetNote) {
    value = channel->m_fPortamentoNote;
  }
  else {
    value = m_midi_note;
  }
  
  // add transpose, detune, pitch bend
  //value = value + (m_detuneFactor * channel->m_fDetuneStep) + channel->m_fPitchBend + conf->transpose + conf->fFineTune;
  
  // add envelope modulation
//  if(conf->modEnv2Pitch) {
//    value = value + m_mod.fValue * conf->modEnv2Pitch/(63.0/12.0);
//  }
  
  // add LFO modulation
//  if(conf->lfo2Pitch) {
//    value = value + channel->m_fLFO * conf->lfo2Pitch/(63.0/12.0);
//  }  
  
  // calculate hz value
  value = 440.0 * pow(2.0,((value-57.0)/12.0));
  
  // apply to POKEY channel
  pokey->pitch(m_voice, value);
  //pokey->pitch(m_voice, 220);


  ////////////////////////////////////////////////////////////////////////////////
  // VOLUME CALCULATION
  ////////////////////////////////////////////////////////////////////////////////

  // apply volume envelope
  value = m_midi_vel/127.0 * m_amp.fValue;

  // apply LFO modulation  
//  if(conf->lfoDest & TONE_CONFIG::TO_VOL) {
//    value *= channel->m_fLFO;
//  }
//  else if(conf->lfoDestNeg & TONE_CONFIG::TO_VOL) {
//    value *= (1.0-channel->m_fLFO);
//  }

  // apply mod wheel modulation
//  if(conf->modWheelDest & TONE_CONFIG::TO_VOL) {
//    value *= channel->m_fModWheel;
//  }
//  else if(conf->modWheelDestNeg & TONE_CONFIG::TO_VOL) {
//    value *= (1.0-channel->m_fModWheel);
//  }
  
  // apply to POKEY channel
  pokey->vol(m_voice, value);
  //pokey->vol(m_voice, 1.0);
  
  /*
  ////////////////////////////////////////////////////////////////////////////////
  // DISTORTION
  ////////////////////////////////////////////////////////////////////////////////
  
  // get initial value
  value = conf->dist/127.0;

  // envelope modulation  
  if(conf->modEnvDest & TONE_CONFIG::TO_DIST) {
    value = value * m_mod.fValue;
  }
  else if(conf->modEnvDestNeg & TONE_CONFIG::TO_DIST) {
    value = value * (1.0 - m_mod.fValue);
  }
  
  // LFO modulation
  if(conf->lfoDest & TONE_CONFIG::TO_DIST) {
    value *= channel->m_fLFO;
  }
  else if(conf->lfoDestNeg & TONE_CONFIG::TO_DIST) {
    value *= (1.0 - channel->m_fLFO);
  }

  // mod wheel
  if(conf->modWheelDest & TONE_CONFIG::TO_DIST) {
    value *= channel->m_fModWheel;
  }
  else if(conf->modWheelDestNeg & TONE_CONFIG::TO_DIST) {
    value *= (1.0-channel->m_fModWheel);
  }
  
  // apply final distortion value to channel
  pokey->dist(m_voice, value);
*/
  ////////////////////////////////////////////////////////////////////////////////
  // HIGH PASS FILTER FREQ CALCULATION
  ////////////////////////////////////////////////////////////////////////////////

/*
  // get initial value
  value = conf->hpf/127.0;

  // apply envelope modulation
  if(conf->modEnvDest & TONE_CONFIG::TO_HPF) {
    value = value * m_mod.fValue;
  }
  else if(conf->modEnvDestNeg & TONE_CONFIG::TO_HPF) {
    value = value * (1.0 - m_mod.fValue);
  }

  // apply LFO modulation
  if(conf->lfoDest & TONE_CONFIG::TO_HPF) {
    value *= channel->m_fLFO;
  }
  else if(conf->lfoDestNeg & TONE_CONFIG::TO_HPF) {
    value *= (1.0 - channel->m_fLFO);
  }  
  
  // apply mod wheel
  if(conf->modWheelDest & TONE_CONFIG::TO_HPF) {
    value *= channel->m_fModWheel;
  }
  else if(conf->modWheelDestNeg & TONE_CONFIG::TO_HPF) {
    value *= (1.0-channel->m_fModWheel);
  }
  
  // apply HPF to the channel
  pokey->hpf(m_voice, value);
*/  
}

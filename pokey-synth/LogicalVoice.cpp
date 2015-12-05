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
  m_detuneFactor = 0;
}

///////////////////////////////////////////////////////////////////////
// ASSIGN POKEY CHANNEL AND LOGICAL CHANNEL TO VOICE
void CLogicalVoice::assign(byte channel, byte voice)
{
  m_channel = channel;  
  m_voice = voice;
}

///////////////////////////////////////////////////////////////////////
// SILENCE THE VOICE
void CLogicalVoice::quiet()
{
  CPokey *pokey = (m_voice & VOICE_POKEY2) ? &Pokey2 : &Pokey1;
  pokey->vol(m_voice, 0);
  
  m_midi_note = 0;
  m_midi_vel = 0;
  m_amp.ePhase = m_mod.ePhase = ENVELOPE_STATE::NONE;
  m_amp.fValue = m_mod.fValue = 0.0;
  m_mod.ePhase = m_mod.ePhase = ENVELOPE_STATE::NONE;
  m_mod.fValue = m_mod.fValue = 0.0;  
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
  float modDepth;

  CLogicalChannel *channel = &Channel[m_channel];
  TONE_CONFIG *conf = channel->m_conf;
  
  ////////////////////////////////////////////////////////////////////////////////
  // PITCH CALCULATION
  ////////////////////////////////////////////////////////////////////////////////
  
  // check if portamento is in progress
  if(channel->m_portaTargetNote) {
    // yes - use the portmento note
    value = channel->m_fPortamentoNote;
  }
  else {
    // no - use the midi note
    value = m_midi_note;
  }
  
  // add transpose, detune, pitch bend
  value = value + (m_detuneFactor * channel->m_fDetuneStep) + channel->m_fPitchBend + conf->transpose + conf->fFineTune;
  
  // The LFO modulates the pitch by simply adding the scaled waveform to it. A full setting of 
  // LFO-to-pitch controller will correspond +1/-1 octave deviation of pitch
  value = value + channel->m_fLFO * conf->ylfo2Pitch/(127.0/12.0);

  // Likewise the mod evelope value is added. A full setting of Mod Env-to-pitch will correspond to
  // +1/-1 octave deviation of pitch
  value = value + m_mod.fValue * conf->ymodEnv2Pitch/(127.0/12.0);
  
  // calculate hz value
  value = 440.0 * pow(2.0,((value-57.0)/12.0));
  
  // apply to POKEY channel
  pokey->pitch(m_voice, value);

  ////////////////////////////////////////////////////////////////////////////////
  // VOLUME CALCULATION
  ////////////////////////////////////////////////////////////////////////////////

  // Fetch the initial volume (scaled to range 0.0-1.0) from either the
  // MIDI note velocity or mod wheel if overriding it
  if(conf->modWheelDest & TONE_CONFIG::TO_VOL) {
    value = channel->m_fModWheel;
  }
  else {
    value = m_midi_vel/127.0;
  }

  // The volume is modulated by the LFO and the amp envelope, which are 
  // mixed according to the LFO-to-Volume depth controller
  modDepth = conf->ylfo2Vol / 127.0;    
  value *= (
      (1.0 - modDepth) * m_amp.fValue + 
      (modDepth * channel->m_fLFO)
  );
  
  // apply to POKEY channel
  pokey->vol(m_voice, value);
  
  ////////////////////////////////////////////////////////////////////////////////
  // DISTORTION
  ////////////////////////////////////////////////////////////////////////////////
  
  // Fetch the distortion value (scaled to range 0.0-1.0) from either the
  // distortion controller or mod wheel if overriding it
  if(conf->modWheelDest & TONE_CONFIG::TO_DIST) {
    value = channel->m_fModWheel;
  }
  else {
    value = conf->dist/127.0;
  }

  // if LFO affects distortion then mix in the 
  // LFO level based on the LFO Depth controller
  if((conf->modEnvDest & TONE_CONFIG::TO_DIST) {
    modDepth = conf->ylfoDepth/127.0;
    value = (
      (1.0 - modDepth) * value + 
      (modDepth * m_mod.fValue)
    );
  }

  // if Mod Env affects distortion then mix in the 
  // Mod Env level based on the Mod Env Depth controller
  // Note - LFO and Mod Env levels are *added* here. For 
  // *scaling* the LFO depth based on the Mod Env there is a
  // Mod Env -> LFO Depth controller!
  if((conf->modEnvDest & TONE_CONFIG::TO_DIST) {
    modDepth = conf->xmodEnvDepth/127.0;
    value = (
      (1.0 - modDepth) * value + 
      (modDepth * m_mod.fValue)
    );
  }

  // apply final distortion value to channel
  pokey->dist(m_voice, value);
  
  ////////////////////////////////////////////////////////////////////////////////
  // HIGH PASS FILTER FREQ CALCULATION
  ////////////////////////////////////////////////////////////////////////////////

  if(conf->modWheelDest & TONE_CONFIG::TO_HPF) {
    value = channel->m_fModWheel;
  }
  else {
    value = conf->hpf/127.0;
  }
  
/*
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
*/  
  // apply HPF to the channel
  pokey->hpf(m_voice, value);
  
}

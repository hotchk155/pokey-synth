///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

//TODO inhibit portamento on poly chann
// LFO modulation on volume - no modulation should = full volume?
//Velocity as modulator
// separate envelopes for modulation and volume

///////////////////////////////////////////////////////////
//
// LOGICAL VOICE
//
// A logical voice is owned by a logical channel, and is 
// itself assigned a physical POKEY voice. The logical voice
// is a proxy to the physical voice - it controls and modulates
// the physical voice
//
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
  m_lch = NULL;
  m_pch = NULL;
  reset();
}

///////////////////////////////////////////////////////////////////////
// ASSIGN POKEY CHANNEL AND LOGICAL CHANNEL TO VOICE
void CLogicalVoice::assign(CLogicalChannel *lch, CPokeyChannel *pch)
{
  m_pch = pch;
  m_lch = lch;  
  reset();
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
// SET THE DIVIDER RANGE
void CLogicalVoice::range(byte v) {
  m_pch->range(v);
}
void CLogicalVoice::poly9(byte v) {
  m_pch->poly9(v);
}

///////////////////////////////////////////////////////////////////////
// UPDATE LOGICAL VOICE
// This is the all important point where the triggers, modulation etc
// get applied to the assigned POKEY voice
void CLogicalVoice::update() 
{    
  if(!m_lch || !m_pch) {
    return;
  }
  float value;
  TONE_CONFIG *conf = m_lch->m_conf;
  
  ////////////////////////////////////////////////////////////////////////////////
  // PITCH CALCULATION
  ////////////////////////////////////////////////////////////////////////////////
  
  // get initial value
  if(m_lch->m_portaTargetNote) {
    value = m_lch->m_fPortamentoNote;
  }
  else {
    value = m_midi_note;
  }
  
  // add transpose, detune, pitch bend
  value = value + (m_detuneFactor * m_lch->m_fDetuneStep) + m_lch->m_fPitchBend + conf->transpose + conf->fFineTune;
  
  // add envelope modulation
  if(conf->modEnv2Pitch) {
    value = value + m_mod.fValue * conf->modEnv2Pitch/(63.0/12.0);
  }
  
  // add LFO modulation
  if(conf->lfo2Pitch) {
    value = value + m_lch->m_fLFO * conf->lfo2Pitch/(63.0/12.0);
  }  
  
  // calculate hz value
  value = 440.0 * pow(2.0,((value-57.0)/12.0));
  
  // apply to POKEY channel
  m_pch->pitch(value);

  ////////////////////////////////////////////////////////////////////////////////
  // VOLUME CALCULATION
  ////////////////////////////////////////////////////////////////////////////////

  // apply volume envelope
  value = m_midi_vel/127.0 * m_amp.fValue;

  // apply LFO modulation  
  if(conf->lfoDest & TONE_CONFIG::TO_VOL) {
    value *= m_lch->m_fLFO;
  }
  else if(conf->lfoDestNeg & TONE_CONFIG::TO_VOL) {
    value *= (1.0-m_lch->m_fLFO);
  }

  // apply mod wheel modulation
  if(conf->modWheelDest & TONE_CONFIG::TO_VOL) {
    value *= m_lch->m_fModWheel;
  }
  else if(conf->modWheelDestNeg & TONE_CONFIG::TO_VOL) {
    value *= (1.0-m_lch->m_fModWheel);
  }
  
  // apply to POKEY channel
  m_pch->vol(0.5 + 15 * value);

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
    value *= m_lch->m_fLFO;
  }
  else if(conf->lfoDestNeg & TONE_CONFIG::TO_DIST) {
    value *= (1.0 - m_lch->m_fLFO);
  }

  // mod wheel
  if(conf->modWheelDest & TONE_CONFIG::TO_DIST) {
    value *= m_lch->m_fModWheel;
  }
  else if(conf->modWheelDestNeg & TONE_CONFIG::TO_DIST) {
    value *= (1.0-m_lch->m_fModWheel);
  }
  
  // apply final distortion value to channel
  m_pch->dist_lev(127.0 * value);

  ////////////////////////////////////////////////////////////////////////////////
  // HIGH PASS FILTER FREQ CALCULATION
  ////////////////////////////////////////////////////////////////////////////////

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
    value *= m_lch->m_fLFO;
  }
  else if(conf->lfoDestNeg & TONE_CONFIG::TO_HPF) {
    value *= (1.0 - m_lch->m_fLFO);
  }  
  
  // apply mod wheel
  if(conf->modWheelDest & TONE_CONFIG::TO_HPF) {
    value *= m_lch->m_fModWheel;
  }
  else if(conf->modWheelDestNeg & TONE_CONFIG::TO_HPF) {
    value *= (1.0-m_lch->m_fModWheel);
  }
  
  // apply HPF to the channel
  m_pch->hpf_lev(255.0 * value);    
}



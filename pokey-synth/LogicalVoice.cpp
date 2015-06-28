///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

//TODO inhibit portamento on poly chann
// LFO modulation on volume - no modulation should = full volume?

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
//
// LOGICAL VOICE
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CLogicalVoice::CLogicalVoice()
{
  m_lch = NULL;
  m_pch = NULL;
  reset();
}

///////////////////////////////////////////////////////////////////////
void CLogicalVoice::reset()
{
  m_note = 0.0;
  m_vol = 0.0;
  m_eEnvelopePhase = CLogicalVoice::ENV_NONE;
  m_fEnvelope = 0;
  m_detuneFactor = 0;
}

///////////////////////////////////////////////////////////////////////
void CLogicalVoice::assign(CLogicalChannel *lch, CPokeyChannel *pch)
{
  m_pch = pch;
  m_lch = lch;  
  reset();
}

///////////////////////////////////////////////////////////////////////
void CLogicalVoice::range(byte v) {
  m_pch->range(v);
}

///////////////////////////////////////////////////////////////////////
// UPDATE LOGICAL VOICE
// This is the all important point where the triggers, modulation etc
// get applied to the assigned POKEY voice
void CLogicalVoice::update() 
{    
  float value;
//  CPokeyChannel *pch = &PokeyChannels[m_pch];
//  CLogicalChannel *lch = &LogicalChannels[m_lch];
//  LOGICAL_CHANNEL *conf = &GlobalConfig.lch[m_lch];
  if(!m_lch || !m_pch) {
    return;
  }
  CHANNEL_CONFIG *conf = m_lch->m_conf;
  if(conf->flags & CF_PORTAMENTO) {
    value = m_lch->m_fPortamentoNote;
  }
  else {
    value = m_note;
  }
  // NOTE
  value = value + (m_detuneFactor * m_lch->m_fDetuneStep) + m_lch->m_fPitchBend + conf->transpose + conf->fineTune;
  if(ENV2PITCH == conf->eLFODest) {
    value = value + 12.0 * m_fEnvelope;
  }
  if(LFO2PITCH == conf->eLFODest) {
    value = value + 12.0 * m_lch->m_fLFOBipolar;
  }
  value = 440.0 * pow(2.0,((value-57.0)/12.0));
  m_pch->pitch(value);

  // VOLUME
  value = m_vol;
  if(ENV2VOL == conf->eEnvelopeDest) {   
    value = value * m_fEnvelope;
  }
  if(LFO2VOL == conf->eLFODest) {   
    value = value * (1.0-m_lch->m_fLFO);
  }
  if(MOD2VOL == conf->eModWheelDest) {       
     value = value * m_lch->m_fModWheel;
  }
  m_pch->vol(0.5 + 15 * value);

  // HPF
  value = conf->hpf/127.0;
  if(ENV2HPF == conf->eEnvelopeDest) {       
    value = value * m_fEnvelope;
  }
  if(LFO2HPF == conf->eLFODest) {       
    value = value * m_lch->m_fLFO;
  }
  if(MOD2HPF == conf->eModWheelDest) {       
     value = value * m_lch->m_fModWheel;
  }
  m_pch->hpf_lev(1000.0 * value);    
  
  // DIST    
  value = conf->dist/127.0;
  if(ENV2DIST == conf->eEnvelopeDest) {       
     value = value * m_fEnvelope;
  }
  if(LFO2DIST == conf->eLFODest) {       
     value = value * m_lch->m_fLFO;
  }
  if(MOD2DIST == conf->eModWheelDest) {       
     value = value * m_lch->m_fModWheel;
  }
  m_pch->dist_lev(127.0 * value);
}



///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
#include "Arduino.h"
#include "Defs.h"
#include "Pokey.h"
#include "LogicalVoice.h"
#include "LogicalChannel.h"


///////////////////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL
//
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
CLogicalChannel::CLogicalChannel() 
{
  m_conf = NULL;
  m_voices = NULL;
  m_voiceCount = 0;
  m_noteCount = 0;
  reset();
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::reset() {

  m_flags = 0;
  m_fPitchBend = 0;
  m_fModWheel = 0;  
  m_fLFO = 0.0;
  m_fLFOBipolar = 0.0;
  m_fPortamentoNote = 0.0;
  m_fDetuneStep = 0.0;  
  m_arpCounter = 0;
  m_arpIndex = 0;
  m_fLFOCount = 0;
  m_portaTargetNote = 0;
  m_fPortaStep = 0; 
  quiet();
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::quiet() {
  m_noteCount = 0;
}

///////////////////////////////////////////////////////////////////////  
void CLogicalChannel::assign(CLogicalVoice *voices, int voiceCount, CHANNEL_CONFIG *conf)
{
  m_conf = conf;
  m_voices = voices;
  m_voiceCount = voiceCount;
  reset();
}
///////////////////////////////////////////////////////////////////////
byte CLogicalChannel::deleteNote(byte note) {
  for(int i=0; i<m_noteCount; ++i) {
    if(m_notes[i].note == note) {
      // remove it
      --m_noteCount;
      for(;i<m_noteCount;++i) {
        m_notes[i] = m_notes[i+1];
      }
      return 1;
    }
  }      
  return 0;
}  
///////////////////////////////////////////////////////////////////////  
void CLogicalChannel::handle(byte status, byte *params)
{
  if(!m_conf) {
    return;
  }
  byte cmd = (status & 0xF0);
  switch(cmd) {    
  case 0x80:
  case 0x90:
    if(cmd == 0x80 || cmd == 0x90) {
      if((status & 0x0F) == m_conf->midiChannel || (m_conf->flags & CF_OMNINOTES)) {
        //          if(params[1] >= m_trigMin && params[1] <= m_trigMax) {
        if(cmd == 0x80 || params[1] == 0x00) {
          handleNoteOff(params[0]);
        }
        else {
          handleNoteOn(params[0], params[1]);
        }        
        //        }
      }
    }
    break;
  case 0xB0: 
    if(((params[1] != CC_OMNI) && (m_conf->flags & CF_OMNICC)) || 
      ((status & 0x0F) == m_conf->midiChannel) ) {
      handleCC(params[0], params[1]);
    }      
    break;
  case 0xE0: 
    {
      if((status & 0x0F) == m_conf->midiChannel || (m_conf->flags & CF_OMNINOTES)) {
        handlePitchBend(params[0], params[1]);
      }      
      break;
    }
  }    
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handleNoteOn(byte note, byte velocity)
{
  // ensure the note is not already in the list (if
  // it is, it will be moved to top of list)
  deleteNote(note);

  // check if the list is out of space
  if(m_noteCount >= MAX_NOTES) {
    for(int i=1; i<m_noteCount; ++i) {
      m_notes[i-1] = m_notes[i];
    }      
  } 
  else {
    ++m_noteCount;
  }
  // place the new note at the top of the stack
  m_notes[m_noteCount-1].note = note;
  m_notes[m_noteCount-1].velocity = velocity;    
  if(!(m_conf->flags & CF_UNISON) && (m_noteCount > m_voiceCount)) {    
    // more notes than voices, so mute the oldest note
    int nextMute = m_noteCount-m_voiceCount-1;
    untrig(m_notes[nextMute].note);
  }
  // trigger the newest note
  trig(note, (m_conf->flags & CF_FULLVELOCITY)? 127 : velocity);
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handleNoteOff(byte note)
{
  untrig(note);
  if(deleteNote(note)) {

    int voiceCount = (m_conf->flags & CF_UNISON)? 1:m_voiceCount;
    if(m_noteCount >= voiceCount) {      
      // all voices were in use.. we might be able to 
      // reactivate a note that was previously overridden
      trig(m_notes[m_noteCount-voiceCount].note,  (m_conf->flags & CF_FULLVELOCITY)? 127 : m_notes[m_noteCount-voiceCount].velocity);
    }
  }
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handlePitchBend(byte lo, byte hi)
{
  m_fPitchBend = ((((int)hi)<<7 | lo) - 8192);
  m_fPitchBend *= m_conf->pitchBendRange;
  m_fPitchBend /= 16384;
}  

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::ccOmni(char value) {
  m_conf->flags &= ~(CF_OMNINOTES|CF_OMNICC);
  if(value < 32) {
  }
  else if(value < 64) {
    m_conf->flags |= CF_OMNINOTES;          
  }
  else if(value < 96) {
    m_conf->flags |= CF_OMNICC;          
  }
  else {
    m_conf->flags |= CF_OMNINOTES|CF_OMNICC;
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::ccFlag(int flag, char value) {
  if(value < 64)
    m_conf->flags &= ~flag;
  else
    m_conf->flags |= flag;
}

///////////////////////////////////////////////////////////////////////
byte CLogicalChannel::ccMapValue(char value, int maxValue) {
  return maxValue * value/128.0;
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handleCC(char cc, char value)
{
  switch(cc) {    
  case CC_OMNI: 
    ccOmni(value); 
    break;        
  case CC_MIDIVEL: 
    ccFlag(CF_FULLVELOCITY, value); 
    break;
  case CC_UNISON: 
    ccFlag(CF_UNISON, value); 
    break;
  case CC_ARPMODE: 
    ccFlag(CF_ARPEGGIATE, value); 
    break;
  case CC_MOD:  
    m_fModWheel = (float)value/127; 
    break;
  case CC_TRIGMIN: 
    m_conf->trigMin = value; 
    break;
  case CC_TRIGMAX: 
    m_conf->trigMax = value; 
    break;
  case CC_TRANSPOSE: 
    m_conf->transpose = value - 64; 
    break;
  case CC_FINETUNE: 
    m_conf->fineTune = ((float)value - 64)/10.0; 
    break;
  case CC_ARPRATE: 
    m_conf->arpPeriod = 127-value; 
    break;
  case CC_ARPCOUNT: 
    m_conf->arpCount = 1 + value/16; 
    break;
  case CC_LFORATE: 
    m_conf->fLFOStep = 1.0/((float)value+1); 
    break;
  case CC_LFOLEVEL: 
    m_conf->fLFOLevel = (float)value/127; 
    break;
  case CC_ENVATTACK: 
    m_conf->fAttackStep = 1.0/((float)value+1); 
    break;
  case CC_ENVRELEASE: 
    m_conf->fReleaseStep = 1.0/((float)value+1); 
    break;
  case CC_PBRANGE: 
    m_conf->pitchBendRange = 12.0 * (value/127.0); 
    break;
  case CC_HPF: 
    m_conf->hpf = 4000.0 * (1.0 -  1.0/((float)value+1));
  case CC_DIST: 
    m_conf->dist = value; 
    break;    
  case CC_DETUNELEVEL: 
    m_conf->detuneLevel = value - 64; 
    break;
  case CC_DETUNEMODE: 
    m_conf->eDetuneMode = ccMapValue(value, DETUNE_MAX); 
    recalc_detune(); 
    break;    
  case CC_MODDEST: 
    m_conf->eModWheelDest = ccMapValue(value, MOD2MAX); 
    break;    
  case CC_LFOMODE: 
    m_conf->eLFOMode = ccMapValue(value, RUN_MAX); 
    break;    
  case CC_LFOWAVE: 
    m_conf->eLFOWave = ccMapValue(value, LFO_MAX); 
    break;    
  case CC_LFODEST: 
    m_conf->eLFODest = ccMapValue(value, LFO2MAX); 
    break;    
  case CC_ENVDEST: 
    m_conf->eEnvelopeDest = ccMapValue(value, ENV2MAX); 
    break;    
  case CC_DIVRANGE: 
    range(value>63); 
    break;    
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::trig(byte note, byte velocity) {    

  switch(m_conf->eLFOMode)
  {
  case RUN_TRIG:
  case RUN_TRIG_GATE:
  case RUN_TRIG_ONCE:
    m_fLFOCount = 0.0;
    m_flags &= ~SF_LFOSIGN;
    m_flags &= ~SF_LFOCOMPLETE;
    break;
  case RUN_GATE:   
  case RUN_UNGATE:
  case RUN_FREE:      
  default:
    break;
  }

  CLogicalVoice *voice = NULL;
  if(m_conf->portaTime) {
    m_portaTargetNote = note;
    if(m_fPortamentoNote > 0) {  
      m_fPortaStep = (note - m_fPortamentoNote) / (m_conf->portaTime * 10.0);
      note = m_fPortamentoNote;
    }
    else {
      m_fPortamentoNote = note;
      m_fPortaStep = 1.0;
    }
  }

  if(m_conf->flags & (CF_UNISON|CF_ARPEGGIATE))
  {
    float n = note;
    for(int i=0; i<m_voiceCount; ++i) {
      voice = &m_voices[i];
      voice->m_note = n;
      voice->m_vol = velocity/127.0;
      voice->m_eEnvelopePhase = CLogicalVoice::ENV_ATTACK;    
      voice->m_fEnvelope = 0.0;
    }
  }
  else
  {
    // check if the note is already assigned to a channel...
    // if it is, then retrigger it
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_note == note) {
        voice = &m_voices[i];
        break;
      }
    }
    if(!voice) {
      // Otherwise do we have a free channel?
      for(int i=0; i<m_voiceCount; ++i) {
        if(m_voices[i].m_eEnvelopePhase == CLogicalVoice::ENV_NONE) {
          voice = &m_voices[i];
          break;
        }
      }
    }
    if(!voice) {
      // or failing that, one that is in release
      for(int i=0; i<m_voiceCount; ++i) {
        if(m_voices[i].m_eEnvelopePhase == CLogicalVoice::ENV_RELEASE) {
          voice = &m_voices[i];
          break;
        }
      }
    }
    if(voice) {
      voice->m_note = note;
      voice->m_vol = velocity/127.0;
      voice->m_eEnvelopePhase = CLogicalVoice::ENV_ATTACK;    
      voice->m_fEnvelope = 0.0;
    }
  }
}  
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::untrig(byte note) {
  if(m_conf->flags & CF_UNISON)
  {
    for(int i=0; i<m_voiceCount; ++i) {
      m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_RELEASE;
      if(m_conf->eEnvelopeDest != ENV2VOL)
        m_voices[i].m_note = 0;
    }
  }
  else
  {  
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_note == note) {
        m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_RELEASE;
        if(m_conf->eEnvelopeDest != ENV2VOL)
          m_voices[i].m_note = 0;
        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::range(byte v) {
  for(int i=0; i<m_voiceCount; ++i) {
    m_voices[i].range(v);
  }
}

///////////////////////////////////////////////////////////////////////
// recalculate voice detune
void CLogicalChannel::recalc_detune()
{
  // calculate the detune for each voice
  for(int i=0; i<m_voiceCount; ++i)
  {
    int mult = 0;
    switch(m_conf->eDetuneMode)
    {
      // these modes spread out in both directions 
      // from central note, with first voice playing
      // unadjusted note
    case DETUNE_FINE:
    case DETUNE_SPREAD:
      if(!i) mult = 0;
      else if(i&1) mult = (i+1)/2;
      else mult = -(i/2);
      break;
      // this mode divides voices between trigger 
      // note and trigger + interval
    case DETUNE_INTERVAL:
      mult = (i&1);
      break;
      // this mode stacks intervals one one top of other 
      // above played note (or below for -ve detune)
    case DETUNE_STACK:
      mult = i;
      break;      
      // detune is disabled
    case DETUNE_NONE:
    default:
      mult = 0;
      break;
    }
    m_voices[i].m_detuneFactor = mult;
  }
}


///////////////////////////////////////////////////////////////////////
// TICK
// called once per millisecond... manages tremelo, vibrato, A/R envelope
void CLogicalChannel::tick(byte counter) 
{
  CLogicalVoice *voice;

  // Every voice has its own envelope, so envelope modulation
  // will only be effective in unison mode where all the envelopes
  // are the same
  float fEnvelope;
  if(m_conf->flags & CF_UNISON) {
    fEnvelope = m_voices[0].m_fEnvelope;
  }
  else {
    fEnvelope = 1.0;
  }

  // the various channel state machines run on different phases 
  // of the tick counter, and each at 8ms intervals
  switch(counter & 0x07)
  {
    /////////////////////////////////////////////
    //
    // ENVELOPE
    //
    /////////////////////////////////////////////  
  case 0:
    // precalculate envelope increments
    for(int i=0; i<m_voiceCount; ++i) {       
      switch(m_voices[i].m_eEnvelopePhase) {                // ATTACK!!
        case CLogicalVoice::ENV_ATTACK:
        m_voices[i].m_fEnvelope += m_conf->fAttackStep;
        if(m_voices[i].m_fEnvelope >= 1.0) {
          m_voices[i].m_fEnvelope = 1.0;
          if(m_flags & CF_ENVLOOP) {
            m_voices[i].m_fEnvelope = 0.0;
            m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_ATTACK;
          }
          else {
            m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_SUSTAIN;
          }
        }
        break;
        case CLogicalVoice::ENV_RELEASE:
        m_voices[i].m_fEnvelope -= m_conf->fReleaseStep;
        if(m_voices[i].m_fEnvelope <= 0.0) {
          m_voices[i].m_fEnvelope = 0.0;
          m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_NONE;
        }
        break;
      }
    }
    break;
    /////////////////////////////////////////////  

    /////////////////////////////////////////////
    //
    // Run LFO  
    //
    /////////////////////////////////////////////  
  case 1:          
    // work out if the LFO counter should be running
    byte lfoRun;
    switch(m_conf->eLFOMode)
    {      
    case RUN_HOLD:
      lfoRun = 0;
      break;
    case RUN_TRIG_ONCE:
      lfoRun = !(m_flags & SF_LFOCOMPLETE);
      break; 
    case RUN_TRIG_GATE:
    case RUN_GATE:   
      lfoRun = !!m_noteCount;
      break;          
    case RUN_UNGATE:
      lfoRun = !m_noteCount;
      break;
    case RUN_TRIG:
    case RUN_FREE:      
    default:
      lfoRun = 1;
      break;
    }    
    if(lfoRun) {        
      // Modulation of the LFO rate
      float fLFOStep = m_conf->fLFOStep;
      if(m_conf->eEnvelopeDest == ENV2LFORATE) {
        fLFOStep *= fEnvelope;
      }
      if(m_conf->eModWheelDest == MOD2LFORATE) {
        fLFOStep *= m_fModWheel;
      }

      // Update the LFO counter
      if(m_flags & SF_LFOSIGN) {
        m_fLFOCount += fLFOStep;
        if(m_fLFOCount >= 1.0) {
          m_fLFOCount = 1.0;
          m_flags &= ~SF_LFOSIGN;
        }
      }
      else {
        m_fLFOCount -= fLFOStep;
        if(m_fLFOCount <= 0.0) {
          m_fLFOCount = 0.0;
          m_flags |= SF_LFOSIGN;
          m_flags |= SF_LFOCOMPLETE;
        }
      }
    }

    switch(m_conf->eLFOWave) 
    {
    case LFO_TRI:  // TRIANGLE
      m_fLFO = m_fLFOCount;
      break;
    case LFO_SQ:   // SQUARE  
      m_fLFO = (m_flags & SF_LFOSIGN)? 0.0 : 1.0;
      break;
    case LFO_RAMP:    // RAMP UP
      m_fLFO = (m_flags & SF_LFOSIGN)? (m_fLFOCount/2.0) : (1.0 - (m_fLFOCount/2.0));
      break;
    case LFO_REVRAMP:    // RAMP DOWN
      m_fLFO = (m_flags & SF_LFOSIGN)? (1.0 - (m_fLFOCount/2.0)) : (m_fLFOCount/2.0);
      break;
    case LFO_RND1:
      if(m_fLFOCount == 0.0) {
        m_fLFO = random(1000) / 1000.0;
      }
      break;
    }      
    m_fLFO *= m_conf->fLFOLevel;
    if(m_conf->eEnvelopeDest == ENV2LFOLEVEL) {
      m_fLFO *= fEnvelope;
    }
    if(m_conf->eModWheelDest == MOD2LFOLEVEL) {
      m_fLFO *= m_fModWheel;
    }

    // calculate bipolar LFO
    m_fLFOBipolar = (2.0*m_fLFO)-1.0;
    break;
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    //
    // Run Portamento
    //
    /////////////////////////////////////////////  
  case 2:

    // Run Portamento
    if((m_conf->flags & CF_PORTAMENTO) && m_portaTargetNote) {
      float d = m_fPortamentoNote + m_fPortaStep;
      if(m_fPortamentoNote > m_portaTargetNote && d < m_portaTargetNote) {
        m_fPortamentoNote = m_portaTargetNote;
        m_portaTargetNote = 0;
      }
      else if(m_fPortamentoNote < m_portaTargetNote && d > m_portaTargetNote) {
        m_fPortamentoNote = m_portaTargetNote;
        m_portaTargetNote = 0;
      }
      else {
        m_fPortamentoNote = d;
      }
    }
    break;
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    //
    // DETUNE STEP RECALCULATION
    //
    /////////////////////////////////////////////  
  case 3:
    switch(m_conf->eDetuneMode) {
    case DETUNE_FINE:  // 10 cents
      m_fDetuneStep = m_conf->detuneLevel/10.0;
      break;
    case DETUNE_INTERVAL:  // semitone
    case DETUNE_SPREAD:
    case DETUNE_STACK:
      m_fDetuneStep = m_conf->detuneLevel;
      break;
    case DETUNE_NONE:
    default:
      m_fDetuneStep = 0.0;
      break;
    }
    if(m_conf->eModWheelDest == MOD2DETUNE) {
      m_fDetuneStep *= m_fModWheel;
    }
    if(m_conf->eEnvelopeDest == ENV2DETUNE) {
      m_fDetuneStep *= fEnvelope;
    }
    if(m_conf->eLFODest == LFO2DETUNE) {
      m_fDetuneStep *= m_fLFO;
    }
    break;

    /////////////////////////////////////////////
    //
    // ARPEGGIATOR
    //
    /////////////////////////////////////////////  
  case 4:  
    if((m_conf->flags & CF_ARPEGGIATE) && (m_noteCount > 0)) {
      if(--m_arpCounter <= 0)
      {          
        if(m_arpIndex >= m_noteCount) { // reached the last note?            
          if(m_noteCount > m_conf->arpCount) { // need to arp just recent notes?
            m_arpIndex = m_noteCount - m_conf->arpCount;
            // 0 1 2 3 4 5 6 7 8 9 (10)
            //             
          } 
          else {
            m_arpIndex = 0;
          }            
        }      
        trig(m_notes[m_arpIndex].note,  (m_conf->flags & CF_FULLVELOCITY)? 127 : m_notes[m_arpIndex].velocity);      
        ++m_arpIndex;
        m_arpCounter = m_conf->arpPeriod;
      }
      else if(m_arpCounter > m_conf->arpPeriod) {
        // in case CC is being swept down from a long value
        m_arpCounter = m_conf->arpPeriod;
      }
    }
    break;    
  }
}




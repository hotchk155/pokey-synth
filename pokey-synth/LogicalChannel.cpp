//TODO inhibit portamento on poly chann
// LFO modulation on volume - no modulation should = full volume?

///////////////////////////////////////////////////////////
//
// POKEYSYNTH 
// hotchk155/2015
//
///////////////////////////////////////////////////////////
#include "Arduino.h"
#include "PokeySynth.h"
#include "Pokey.h"
#include "LogicalChannel.h"

///////////////////////////////////////////////////////////////////////
//
// LOGICAL VOICE
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CLogicalVoice::CLogicalVoice()
{
  m_pch = NULL;
  m_note = 0.0;
  m_vol = 0.0;
  m_eEnvelopePhase = CLogicalVoice::ENV_NONE;
  m_fEnvelope = 0;
  m_detuneFactor = 0;
}
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::assign(CPokeyChannel *pch) 
{
  m_pch = pch;
} 

///////////////////////////////////////////////////////////////////////
void CLogicalVoice::range(byte v) {
  if(m_pch) {
    m_pch->range(v);
  }
}

///////////////////////////////////////////////////////////////////////
// UPDATE LOGICAL VOICE
// This is the all important point where the triggers, modulation etc
// get applied to the assigned POKEY voice
void CLogicalVoice::update(CLogicalChannel *lch) 
{  
  if(!m_pch)
    return;
  float value;
///////  
//value = 440.0 * pow(2.0,((m_note-57.0)/12.0));
//m_pch->pitch(value);
//m_pch->vol(15);
//return;
///////  
 
  if(lch->m_flags & CLogicalChannel::FLAG_PORTAMENTO) {
    value = lch->m_fPortamentoNote;
  }
  else {
    value = m_note;
  }
//  if(value>1) {   
    
    // NOTE
    value = value + (m_detuneFactor * lch->m_fDetuneStep) + lch->m_fPitchBend + lch->m_transpose + lch->m_fineTune;
    if(CLogicalChannel::ENV2PITCH == lch->m_eLFODest) {
      value = value + 12.0 * m_fEnvelope;
    }
    if(CLogicalChannel::LFO2PITCH == lch->m_eLFODest) {
      value = value + 12.0 * lch->m_fLFOBipolar;
    }
    value = 440.0 * pow(2.0,((value-57.0)/12.0));
    m_pch->pitch(value);

    // VOLUME
    value = m_vol;
    if(CLogicalChannel::ENV2VOL == lch->m_eEnvelopeDest) {   
      value = value * m_fEnvelope;
    }
    if(CLogicalChannel::LFO2VOL == lch->m_eLFODest) {   
      value = value * (1.0-lch->m_fLFO);
    }
    if(CLogicalChannel::MOD2VOL == lch->m_eModWheelDest) {       
       value = value * lch->m_fModWheel;
    }
    m_pch->vol(0.5 + 15 * value);

    // HPF
    value = lch->m_hpf/127.0;
    if(CLogicalChannel::ENV2HPF == lch->m_eEnvelopeDest) {       
      value = value * m_fEnvelope;
    }
    if(CLogicalChannel::LFO2HPF == lch->m_eLFODest) {       
      value = value * lch->m_fLFO;
    }
    if(CLogicalChannel::MOD2HPF == lch->m_eModWheelDest) {       
       value = value * lch->m_fModWheel;
    }
    m_pch->hpf_lev(1000.0 * value);    
    
    // DIST    
    value = lch->m_dist/127.0;
    if(CLogicalChannel::ENV2DIST == lch->m_eEnvelopeDest) {       
       value = value * m_fEnvelope;
    }
    if(CLogicalChannel::LFO2DIST == lch->m_eLFODest) {       
       value = value * lch->m_fLFO;
    }
    if(CLogicalChannel::MOD2DIST == lch->m_eModWheelDest) {       
       value = value * lch->m_fModWheel;
    }
    m_pch->dist_lev(127.0 * value);
//  }
//  else {
//    m_pch->vol(0);
//  }
}

///////////////////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CLogicalChannel::CLogicalChannel(char ch) 
{
  m_voices = NULL;
  m_midiChannel = ch;
  reset();
  /*
  m_flags = FLAG_UNISON|FLAG_FULLVELOCITY;
  m_voices = NULL;
  m_midiChannel = OMNI;
  m_noteCount = 0;
  m_voiceCount = 0;
  m_bendRange = 3;
  m_bend = 0;
  m_transpose = 0;
  m_fineTune = 0;
  
  m_attack = 65535;
  m_release = 60;
  m_envDest = ENV2VOL;
  
  m_portaLevel = 0;
  m_portaTarget = 0;
  m_portamento = 0.0;
  m_portaStep = 0.0;

  m_lfoStep = 2000;
  m_lfoCount = 0;
  m_lfoWave = LFO_SQ;
  m_lfoRun = RUN_GATE;

  m_hpf = 0;
  m_dist = 0;

  m_lfoLevel = 127;
  m_lfo = 0;
  m_lfoDest = LFO2HPF;
    
//  m_tremLevel = 0;
//  m_tremelo = 0;      

//  m_vibLevel = 0;
//  m_vibrato = 127.0;      

  m_arpPeriod = 10;
  m_arpIndex = 0;
  m_arpCounter = 0;
  */
}


///////////////////////////////////////////////////////////////////////  
void CLogicalChannel::init(int voices)
{
  if(m_voices) {
    delete[] m_voices;      
  }
  m_voiceCount = voices;
  m_voices = new CLogicalVoice[voices];
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::assign(int voice, CPokeyChannel *pch)
{
  if(!m_voices || voice < 0 || voice >= m_voiceCount)
    return;
  m_voices[voice].assign(pch);
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
  if(!m_voices || !params)
    return;          
  byte cmd = (status & 0xF0);
  switch(cmd) {    
    case 0x80:
    case 0x90:
      if(cmd == 0x80 || cmd == 0x90) {
        if((status & 0x0F) == m_midiChannel || (m_flags & FLAG_OMNINOTES)) {
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
      if(((params[1] != CC_OMNI) && (m_flags & FLAG_OMNICC)) || 
          ((status & 0x0F) == m_midiChannel) ) {
        handleCC(params[0], params[1]);
      }      
      break;
    case 0xE0: {
      if((status & 0x0F) == m_midiChannel || (m_flags & FLAG_OMNINOTES)) {
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
  if(!(m_flags & FLAG_UNISON) && (m_noteCount > m_voiceCount)) {    
    // more notes than voices, so mute the oldest note
    int nextMute = m_noteCount-m_voiceCount-1;
    untrig(m_notes[nextMute].note);
  }
  // trigger the newest note
  trig(note, (m_flags & FLAG_FULLVELOCITY)? 127 : velocity);
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handleNoteOff(byte note)
{
  untrig(note);
  if(deleteNote(note)) {
    
    int voiceCount = (m_flags & FLAG_UNISON)? 1:m_voiceCount;
    if(m_noteCount >= voiceCount) {      
      // all voices were in use.. we might be able to 
      // reactivate a note that was previously overridden
      trig(m_notes[m_noteCount-voiceCount].note,  (m_flags & FLAG_FULLVELOCITY)? 127 : m_notes[m_noteCount-voiceCount].velocity);
    }
  }
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handlePitchBend(byte lo, byte hi)
{
  m_fPitchBend = ((((int)hi)<<7 | lo) - 8192);
  m_fPitchBend *= m_pitchBendRange;
  m_fPitchBend /= 16384;
  for(int i=0; i<m_voiceCount; ++i) {
    if(m_voices[i].m_note) {
      m_voices[i].update(this);    
    }
  }    
}  

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::ccOmni(char value) {
   m_flags &= ~(FLAG_OMNINOTES|FLAG_OMNICC);
    if(value < 32) {
    }
    else if(value < 64) {
      m_flags |= FLAG_OMNINOTES;          
    }
    else if(value < 96) {
      m_flags |= FLAG_OMNICC;          
    }
    else {
     m_flags |= FLAG_OMNINOTES|FLAG_OMNICC;
    }
 }

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::ccFlag(int flag, char value) {
  if(value < 64)
    m_flags &= ~flag;
  else
    m_flags |= flag;
}

///////////////////////////////////////////////////////////////////////
byte CLogicalChannel::ccMapValue(char value, int maxValue) {
  return maxValue * value/128.0;
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handleCC(char cc, char value)
{
  switch(cc) {    
    case CC_OMNI: ccOmni(value); break;        
    case CC_MIDIVEL: ccFlag(FLAG_FULLVELOCITY, value); break;
    case CC_UNISON: ccFlag(FLAG_UNISON, value); break;
    case CC_ARPMODE: ccFlag(FLAG_ARPEGGIATE, value); break;
    case CC_MOD:  m_fModWheel = (float)value/127; break;
    case CC_TRIGMIN: m_trigMin = value; break;
    case CC_TRIGMAX: m_trigMax = value; break;
    case CC_TRANSPOSE: m_transpose = value - 64; break;
    case CC_FINETUNE: m_fineTune = ((float)value - 64)/10.0; break;
    case CC_ARPRATE: m_arpPeriod = 127-value; break;
    case CC_ARPCOUNT: m_arpCount = 1 + value/16; break;
    case CC_LFORATE: m_fLFOStep = 1.0/((float)value+1); break;
    case CC_LFOLEVEL: m_fLFOLevel = (float)value/127; break;
    case CC_ENVATTACK: m_fAttackStep = 1.0/((float)value+1); break;
    case CC_ENVRELEASE: m_fReleaseStep = 1.0/((float)value+1); break;
    case CC_PBRANGE: m_pitchBendRange = 12.0 * (value/127.0); break;
    case CC_HPF: m_hpf = 4000.0 * (1.0 -  1.0/((float)value+1));
    case CC_DIST: m_dist = value; break;    
    case CC_DETUNELEVEL: m_detuneLevel = value - 64; break;
    case CC_DETUNEMODE: m_eDetuneMode = ccMapValue(value, DETUNE_MAX); recalc_detune(); break;    
    case CC_MODDEST: m_eModWheelDest = ccMapValue(value, MOD2MAX); break;    
    case CC_LFOMODE: m_eLFOMode = ccMapValue(value, RUN_MAX); break;    
    case CC_LFOWAVE: m_eLFOWave = ccMapValue(value, LFO_MAX); break;    
    case CC_LFODEST: m_eLFODest = ccMapValue(value, LFO2MAX); break;    
    case CC_ENVDEST: m_eEnvelopeDest = ccMapValue(value, ENV2MAX); break;    
    case CC_DIVRANGE: range(value>63); break;    
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::trig(byte note, byte velocity) {    

  switch(m_eLFOMode)
  {
    case RUN_TRIG:
    case RUN_TRIG_GATE:
    case RUN_TRIG_ONCE:
      m_fLFOCount = 0.0;
      m_flags &= ~FLAG_LFOSIGN;
      m_flags &= ~FLAG_LFOCOMPLETE;
      break;
    case RUN_GATE:   
    case RUN_UNGATE:
    case RUN_FREE:      
    default:
      break;
  }
  
  CLogicalVoice *voice = NULL;
  if(m_portaTime) {
    m_portaTargetNote = note;
    if(m_fPortamentoNote > 0) {  
      m_fPortaStep = (note - m_fPortamentoNote) / (m_portaTime * 10.0);
      note = m_fPortamentoNote;
    }
    else {
      m_fPortamentoNote = note;
      m_fPortaStep = 1.0;
    }
  }
  
  if(m_flags & (FLAG_UNISON|FLAG_ARPEGGIATE))
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
  if(m_flags & FLAG_UNISON)
  {
    for(int i=0; i<m_voiceCount; ++i) {
      m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_RELEASE;
      if(m_eEnvelopeDest != ENV2VOL)
        m_voices[i].m_note = 0;
    }
  }
  else
  {  
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_note == note) {
        m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_RELEASE;
        if(m_eEnvelopeDest != ENV2VOL)
          m_voices[i].m_note = 0;
        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////
// TICK
// called once per millisecond... manages tremelo, vibrato, A/R envelope
void CLogicalChannel::tick(byte counter) 
{
  // Every voice has its own envelope, so envelope modulation
  // will only be effective in unison mode where all the envelopes
  // are the same
  float fEnvelope;
  if(m_flags & FLAG_UNISON) {
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
            m_voices[i].m_fEnvelope += m_fAttackStep;
            if(m_voices[i].m_fEnvelope >= 1.0) {
              m_voices[i].m_fEnvelope = 1.0;
              if(m_flags & FLAG_ENVLOOP) {
                m_voices[i].m_fEnvelope = 0.0;
                m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_ATTACK;
              }
              else {
                m_voices[i].m_eEnvelopePhase = CLogicalVoice::ENV_SUSTAIN;
              }
            }
            break;
          case CLogicalVoice::ENV_RELEASE:
            m_voices[i].m_fEnvelope -= m_fReleaseStep;
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
      switch(m_eLFOMode)
      {      
        case RUN_HOLD:
          lfoRun = 0;
          break;
        case RUN_TRIG_ONCE:
          lfoRun = !(m_flags & FLAG_LFOCOMPLETE);
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
        float fLFOStep = m_fLFOStep;
        if(m_eEnvelopeDest == ENV2LFORATE) {
          fLFOStep *= fEnvelope;
        }
        if(m_eModWheelDest == MOD2LFORATE) {
          fLFOStep *= m_fModWheel;
        }
    
        // Update the LFO counter
        if(m_flags & FLAG_LFOSIGN) {
          m_fLFOCount += fLFOStep;
          if(m_fLFOCount >= 1.0) {
            m_fLFOCount = 1.0;
            m_flags &= ~FLAG_LFOSIGN;
          }
        }
        else {
          m_fLFOCount -= fLFOStep;
          if(m_fLFOCount <= 0.0) {
            m_fLFOCount = 0.0;
            m_flags |= FLAG_LFOSIGN;
            m_flags |= FLAG_LFOCOMPLETE;
          }
        }
      }
      
      switch(m_eLFOWave) 
      {
        case LFO_TRI:  // TRIANGLE
          m_fLFO = m_fLFOCount;
          break;
        case LFO_SQ:   // SQUARE  
          m_fLFO = (m_flags & FLAG_LFOSIGN)? 0.0 : 1.0;
          break;
        case LFO_RAMP:    // RAMP UP
          m_fLFO = (m_flags & FLAG_LFOSIGN)? (m_fLFOCount/2.0) : (1.0 - (m_fLFOCount/2.0));
          break;
        case LFO_REVRAMP:    // RAMP DOWN
          m_fLFO = (m_flags & FLAG_LFOSIGN)? (1.0 - (m_fLFOCount/2.0)) : (m_fLFOCount/2.0);
          break;
        case LFO_RND1:
            if(m_fLFOCount == 0.0) {
              m_fLFO = random(1000) / 1000.0;
            }
            break;
      }      
      m_fLFO *= m_fLFOLevel;
      if(m_eEnvelopeDest == ENV2LFOLEVEL) {
        m_fLFO *= fEnvelope;
      }
      if(m_eModWheelDest == MOD2LFOLEVEL) {
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
      if((m_flags & FLAG_PORTAMENTO) && m_portaTargetNote) {
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
      switch(m_eDetuneMode) {
        case DETUNE_FINE:  // 10 cents
          m_fDetuneStep = m_detuneLevel/10.0;
          break;
        case DETUNE_INTERVAL:  // semitone
        case DETUNE_SPREAD:
        case DETUNE_STACK:
          m_fDetuneStep = m_detuneLevel;
          break;
        case DETUNE_NONE:
        default:
          m_fDetuneStep = 0.0;
          break;
      }
      if(m_eModWheelDest == MOD2DETUNE) {
        m_fDetuneStep *= m_fModWheel;
      }
      if(m_eEnvelopeDest == ENV2DETUNE) {
        m_fDetuneStep *= fEnvelope;
      }
      if(m_eLFODest == LFO2DETUNE) {
        m_fDetuneStep *= m_fLFO;
      }
      break;
    
    /////////////////////////////////////////////
    //
    // ARPEGGIATOR
    //
    /////////////////////////////////////////////  
    case 4:  
      if((m_flags & FLAG_ARPEGGIATE) && (m_noteCount > 0)) {
        if(--m_arpCounter <= 0)
        {          
          if(m_arpIndex >= m_noteCount) { // reached the last note?            
            if(m_noteCount > m_arpCount) { // need to arp just recent notes?
              m_arpIndex = m_noteCount - m_arpCount;
              // 0 1 2 3 4 5 6 7 8 9 (10)
              //             
            } else {
              m_arpIndex = 0;
            }            
          }      
          trig(m_notes[m_arpIndex].note,  (m_flags & FLAG_FULLVELOCITY)? 127 : m_notes[m_arpIndex].velocity);      
          ++m_arpIndex;
          m_arpCounter = m_arpPeriod;
        }
        else if(m_arpCounter > m_arpPeriod) {
          // in case CC is being swept down from a long value
          m_arpCounter = m_arpPeriod;
        }
      }
      break;    
  }
  
    m_voices[m_updateChan].update(this);
    if(++m_updateChan >= m_voiceCount) 
      m_updateChan = 0;
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
    switch(m_eDetuneMode)
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
void CLogicalChannel::reset() {
  m_flags = FLAG_FULLVELOCITY|FLAG_OMNINOTES|FLAG_OMNICC;
  
  m_trigMin = 0;
  m_trigMax = 127;
  m_transpose = 0;
  m_fineTune = 0;
  m_fPitchBend = 0;
  m_fModWheel = 0;
  m_pitchBendRange = 2;
  m_portaTime = 0;
  m_detuneLevel = 0;
  m_eDetuneMode = 0;
  m_hpf = 0;
  m_dist = 127;
  m_eLFOMode = RUN_FREE;
  m_fLFOStep = 0.0625;
  m_fLFOLevel = 1.0;
  m_eLFOWave = LFO_TRI;
  m_fAttackStep = 1.0;
  m_fReleaseStep = 1.0;
  m_arpPeriod = 20;
  m_arpCount = 8;  
  m_eModWheelDest = MOD2DIST;
  m_eEnvelopeDest = ENV2VOL;
  m_eLFODest = LFO2NONE;
  
  m_fLFO = 0.0;
  m_fLFOBipolar = 0.0;
  m_fPortamentoNote = 0.0;
  m_fDetuneStep = 0.0;
  
  m_arpCounter = 0;
  m_arpIndex = 0;
  m_fLFOCount = 0;
  m_portaTargetNote = 0;
  m_fPortaStep = 0; 
  m_updateChan = 0;
}


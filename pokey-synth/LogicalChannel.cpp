///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL
//
///////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "Defs.h"
#include "Pokey.h"
#include "LogicalVoice.h"
#include "LogicalChannel.h"



///////////////////////////////////////////////////////////////////////
inline float envStep(char v) {
  if(!v) return 1.0;
  return 0.5/((float)v+1);
}

///////////////////////////////////////////////////////////////////////
inline void trigEnvelope(ENVELOPE *env, ENVELOPE_STATE *envState) {
  envState->ePhase = ENVELOPE_STATE::ATTACK;    
  envState->fValue = 0;
}

///////////////////////////////////////////////////////////////////////
inline void untrigEnvelope(ENVELOPE *env, ENVELOPE_STATE *envState) {
  switch(envState->ePhase) {
    case ENVELOPE_STATE::ATTACK:
    case ENVELOPE_STATE::SUSTAIN:
      envState->ePhase = ENVELOPE_STATE::RELEASE;
      break;
  }  
}

///////////////////////////////////////////////////////////////////////
// result is 1 if envelope has finished
inline byte runEnvelope(ENVELOPE *env, ENVELOPE_STATE *envState) {  
  switch(envState->ePhase) {               
    case ENVELOPE_STATE::ATTACK:
        envState->fValue += env->fAttackStep;
        if(envState->fValue >= 1.0) {
          envState->fValue = 1.0;
          // reached end of attack phase... what happens next depends on mode
          switch(env->mode) {            
            case ENVELOPE::ATT_DEC:
            case ENVELOPE::ATT_DEC_RPT_REL:
            case ENVELOPE::LOOP:
              envState->ePhase = ENVELOPE_STATE::RELEASE;
              break;
            case ENVELOPE::ATT_RPT_REL:
              envState->fValue = 0.0;
              envState->ePhase = ENVELOPE_STATE::ATTACK;
            case ENVELOPE::ATT_REL:
            default:
              envState->ePhase = ENVELOPE_STATE::SUSTAIN;
              break;
          }
        }
        break;
    case ENVELOPE_STATE::RELEASE:
        envState->fValue -= env->fReleaseStep;
        if(envState->fValue <= 0.0) {
          envState->fValue = 0.0;
          // reached end of release phase... what happens next depends on mode
          switch(env->mode) {                        
            case ENVELOPE::LOOP:
              envState->ePhase = ENVELOPE_STATE::ATTACK;
              break;
            case ENVELOPE::ATT_RPT_REL:
            case ENVELOPE::ATT_DEC:
            case ENVELOPE::ATT_REL:
            case ENVELOPE::ATT_DEC_RPT_REL:
            default:
              envState->ePhase = ENVELOPE_STATE::NONE;
              return 1;
          }
        }
        break; 
      }
      return 0;
}

///////////////////////////////////////////////////////////////////////
// Constructor
CLogicalChannel::CLogicalChannel() 
{
  m_conf = NULL;
  m_voices = NULL;
  m_voiceCount = 0;
  m_midiChannel = 0;
  reset();
}

///////////////////////////////////////////////////////////////////////  
void CLogicalChannel::test() {
  m_voices[0].test();
}

///////////////////////////////////////////////////////////////////////  
// Assign logical voices and configuration to the channel
void CLogicalChannel::assign(CLogicalVoice *voices, int voiceCount, TONE_CONFIG *conf)
{
  m_voices = voices;
  m_voiceCount = voiceCount;
  m_conf = conf;
}

///////////////////////////////////////////////////////////////////////
// RESET 
// Resets channel state
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
// QUIET
void CLogicalChannel::quiet() {
  // clear the note stack
  m_noteCount = 0;
}

///////////////////////////////////////////////////////////////////////
// SET A FLAG BIT BASED ON CC VALUE
void CLogicalChannel::ccFlag(byte *flags, int flag, char value) {
  if(value < 64)
    *flags &= ~flag;
  else
    *flags |= flag;
}

///////////////////////////////////////////////////////////////////////
// SET A FLAG BIT BASED ON CC VALUE
void CLogicalChannel::ccFlag(byte *flags, byte *negFlags, int flag, char value) {
  if(value < 43) {
    *negFlags |= flag;
    *flags &= ~flag;
  }
  else if(value > 84) {
    *negFlags &= ~flag;
    *flags |= flag;
  }
  else {
    *flags &= ~flag;
    *negFlags &= ~flag;
  }
}

///////////////////////////////////////////////////////////////////////
// MAP A CC VALUE (0-127) TO A SET OF INTEGERS
byte CLogicalChannel::ccMapValue(char value, int maxValue) {
  return maxValue * value/128.0;
}


///////////////////////////////////////////////////////////////////////
void CLogicalChannel::trig(byte note, byte velocity, byte trigEnv) {    

  switch(m_conf->eLFOMode)
  {
  case TONE_CONFIG::LFO_TRIG_FREE:
  case TONE_CONFIG::LFO_TRIG_GATE:
  case TONE_CONFIG::LFO_ONE_SHOT:
    m_fLFOCount = 0.0;
    m_flags &= ~SF_LFOSIGN;
    m_flags &= ~SF_LFOCOMPLETE;
    break;
  case TONE_CONFIG::LFO_GATE:   
  case TONE_CONFIG::LFO_UNGATE:
  case TONE_CONFIG::LFO_FREE:      
  default:
    break;
  }

  CLogicalVoice *voice = NULL;

  if(m_conf->flags & (TONE_CONFIG::UNISON|TONE_CONFIG::ARPEGGIATE))
  {
    // check if a nonzero portamento setting is defined
    if(m_conf->portaTime) {
      
      // store trigggered note as the target of the portamento engine
      m_portaTargetNote = note;
      
      // is there a previous note to use as start point?
      if(m_fPortamentoNote > 0 && note != m_fPortamentoNote) {  
        
        // calculate step rate based on distance between the old and new note
        // and the portamento time setting        
        m_fPortaStep = (note - m_fPortamentoNote) / (float)m_conf->portaTime;
      }
      else {
        // just remember the note for next time!
        m_fPortamentoNote = note;
        m_portaTargetNote = 0;
      }
    }
    else  {
      m_portaTargetNote = 0;
    }

    for(int i=0; i<m_voiceCount; ++i) {
      voice = &m_voices[i];
      voice->m_midi_note = note;
      voice->m_midi_vel = velocity;
      if(trigEnv) {
        trigEnvelope(&m_conf->ampEnv, &voice->m_amp);
        trigEnvelope(&m_conf->modEnv, &voice->m_mod);
      }
    }
  }
  else
  {
    // check if the note is already assigned to a channel...
    // if it is, then retrigger it
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_midi_note == note) {
        voice = &m_voices[i];
        break;
      }
    }
    if(!voice) {
      // Otherwise do we have a free channel?
      for(int i=0; i<m_voiceCount; ++i) {
        if(m_voices[i].m_amp.ePhase == ENVELOPE_STATE::NONE) {
          voice = &m_voices[i];
          break;
        }
      }
    }
    if(!voice) {
      // or failing that, one that is in release
      for(int i=0; i<m_voiceCount; ++i) {
        if(m_voices[i].m_amp.ePhase == ENVELOPE_STATE::RELEASE) {
          voice = &m_voices[i];
          break;
        }
      }
    }
    if(voice) {
      voice->m_midi_note = note;
      voice->m_midi_vel = velocity;
      if(trigEnv) {
        trigEnvelope(&m_conf->ampEnv, &voice->m_amp);
        trigEnvelope(&m_conf->modEnv, &voice->m_mod);
      }
    }
  }
}  

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::untrig(byte note) {
  for(int i=0; i<m_voiceCount; ++i) {
    if(m_voices[i].m_midi_note == note) {      
      untrigEnvelope(&m_conf->ampEnv, &m_voices[i].m_amp);
      untrigEnvelope(&m_conf->modEnv, &m_voices[i].m_mod);
    }
  }
}

///////////////////////////////////////////////////////////////////////
// DELETE NOTE FROM NOTE STACK
byte CLogicalChannel::deleteNote(byte note) {
  for(int i=0; i<m_noteCount; ++i) {
    if(m_notes[i].note == note) {
      // remove it
      --m_noteCount;
      for(;i<m_noteCount;++i) {
        m_notes[i] = m_notes[i+1];
      }
      return 1; // note was found
    }
  }      
  return 0;
}  

///////////////////////////////////////////////////////////////////////
// HANDLE NOTE ON MESSAGE
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
  if(!(m_conf->flags & TONE_CONFIG::UNISON) && (m_noteCount > m_voiceCount)) {    
    // more notes than voices, so mute the oldest note
    int nextMute = m_noteCount-m_voiceCount-1;
    untrig(m_notes[nextMute].note);
  }
  // trigger the newest note
  trig(note, (m_conf->flags & TONE_CONFIG::USE_VELOCITY)? velocity:127, true);
}

///////////////////////////////////////////////////////////////////////
// HANDLE NOTE OFF MESSAGE
void CLogicalChannel::handleNoteOff(byte note)
{
  untrig(note);
  if(deleteNote(note)) {

    int voiceCount = (m_conf->flags & TONE_CONFIG::UNISON)? 1:m_voiceCount;
    if(m_noteCount >= voiceCount) {      
      // all voices were in use.. we might be able to 
      // reactivate a note that was previously overridden
      trig(m_notes[m_noteCount-voiceCount].note,  (m_conf->flags & TONE_CONFIG::USE_VELOCITY)? m_notes[m_noteCount-voiceCount].velocity : 127, true);
    }
  }
}

///////////////////////////////////////////////////////////////////////
// HANDLE PITCH BEND MESSAGE
void CLogicalChannel::handlePitchBend(byte lo, byte hi)
{
  m_fPitchBend = ((((int)hi)<<7 | lo) - 8192);
  m_fPitchBend *= m_conf->pitchBendRange;
  m_fPitchBend /= 16384;
}  

///////////////////////////////////////////////////////////////////////
//
void CLogicalChannel::handleCC(char cc, char value)
{
  switch(cc) {    
  case CC_POKEYCFG:
    {
      byte mode;
      switch(ccMapValue(value, TONE_CONFIG::POKEY_MAX)) {
        case TONE_CONFIG::POKEY_8_HPF:
          mode = CPokey::PCFG_8HPF;
          break;        
        case TONE_CONFIG::POKEY_16:
          mode = CPokey::PCFG_16;
          break;        
        case TONE_CONFIG::POKEY_8:  
        default:
          mode = CPokey::PCFG_8;
          break;
      }
      if(mode != m_conf->ePokeyMode) {
        m_conf->ePokeyMode = mode;
        m_flags |= SF_RECONFIG;
      }
    }
    break;
  case CC_POKEYDUAL:
    {
      byte flags = m_conf->flags;
      ccFlag(&flags, TONE_CONFIG::POKEY_DUAL, value); 
      if(flags != m_conf->flags) {
        m_flags |= SF_RECONFIG;
        m_conf->flags = flags;
      }
    }
    break;
//  case CC_POKEYPOLY9:
//    ccFlag(&m_conf->flags, TONE_CONFIG::POKEY_POLY9, value); 
//    poly9(m_conf->flags & TONE_CONFIG::POKEY_POLY9); 
//    break;    
  case CC_POKEYRANGE: 
    ccFlag(&m_conf->flags, TONE_CONFIG::POKEY_HIHZ, value); 
    range(!!(m_conf->flags & TONE_CONFIG::POKEY_HIHZ)); 
    break;    
  case CC_MIDIVEL: 
    ccFlag(&m_conf->flags, TONE_CONFIG::USE_VELOCITY, value); 
    break;
  case CC_UNISON: 
    ccFlag(&m_conf->flags, TONE_CONFIG::UNISON, value); 
    break;
  case CC_ARPENABLE: 
    ccFlag(&m_conf->flags, TONE_CONFIG::ARPEGGIATE, value); 
    break;
  case CC_ARP2ENV: 
    ccFlag(&m_conf->flags, TONE_CONFIG::ARP2ENV, value); 
    break;
  case CC_MOD:  
    m_fModWheel = (float)value/127; 
    break;
  case CC_PORTATIME: 
    m_conf->portaTime = value; 
    break;  
  case CC_TRANSPOSE: 
    m_conf->transpose = value - 64; 
    break;
  case CC_FINETUNE: 
    m_conf->fFineTune = ((float)value - 64)/10.0; 
    break;
  case CC_ARPRATE: 
    m_conf->arpPeriod = 127-value; 
    break;
  case CC_ARPCOUNT: 
    m_conf->arpCount = 1 + value/16; 
    break;
  case CC_LFORATE: 
    m_conf->fLFOStep = 1.0/(128.0-value); 
    break;
  case CC_AENVATTACK: 
    m_conf->ampEnv.fAttackStep = envStep(value);
    break;
  case CC_AENVRELEASE: 
    m_conf->ampEnv.fReleaseStep = envStep(value);
    break;
  case CC_AENVMODE:
    m_conf->ampEnv.mode = ccMapValue(value, ENVELOPE::MAX_MODE); 
    break;
  case CC_MENVATTACK: 
    m_conf->modEnv.fAttackStep = envStep(value);
    break;
  case CC_MENVRELEASE: 
    m_conf->modEnv.fReleaseStep = envStep(value);
    break;
  case CC_MENVMODE: 
    m_conf->modEnv.mode = ccMapValue(value, ENVELOPE::MAX_MODE); 
    break;    
  case CC_PBRANGE: 
    m_conf->pitchBendRange = 12.0 * (value/127.0); 
    break;
  case CC_HPF: 
    m_conf->hpf = 127-value;
    break;
  case CC_DIST: 
    m_conf->dist = value; 
    break;    
  case CC_DETUNELEVEL: 
    m_conf->detuneLevel = value - 64; 
    break;
  case CC_DETUNEMODE: 
    m_conf->eDetuneMode = ccMapValue(value, TONE_CONFIG::DETUNE_MAX); 
    recalc_detune(); 
    break;    
  case CC_LFOMODE: 
    m_conf->eLFOMode = ccMapValue(value, TONE_CONFIG::LFO_MAX); 
    break;    
  case CC_LFOWAVE: 
    m_conf->eLFOWave = ccMapValue(value, TONE_CONFIG::WAVE_MAX); 
    break;    
      
  // MOD MATRIX SETTINGS FOR LFO
  case CC_LFO_2_PITCH:
    m_conf->lfo2Pitch = value-64; 
    break;    
  case CC_LFO_2_VOL:
    m_conf->lfo2Vol = value-64; 
    break;
  case CC_LFO_2_MATRIX:
    m_conf->lfoDepth = value; 
    break;        
  case CC_LFO_2_DIST:
    ccFlag(&m_conf->lfoDest, &m_conf->lfoDestNeg, TONE_CONFIG::TO_DIST, value); 
    break;    
  case CC_LFO_2_HPF:
    ccFlag(&m_conf->lfoDest, &m_conf->lfoDestNeg, TONE_CONFIG::TO_HPF, value); 
    break;    
  case CC_LFO_2_DETUNE:
    ccFlag(&m_conf->lfoDest, &m_conf->lfoDestNeg, TONE_CONFIG::TO_DETUNE, value); 
    break;    
  case CC_LFO_2_ARP_RATE:
    ccFlag(&m_conf->lfoDest, &m_conf->lfoDestNeg, TONE_CONFIG::TO_ARP_RATE, value); 
    break;    

  // MOD MATRIX SETTINGS FOR MOD ENVELOPE          
  case CC_ENV_2_PITCH:
    m_conf->modEnv2Pitch = value-64; 
    break;    
  case CC_ENV_2_MATRIX:
    m_conf->modEnvDepth = value-64; 
    break;    
  case CC_ENV_2_DISTORTION:
    ccFlag(&m_conf->modEnvDest, &m_conf->modEnvDestNeg, TONE_CONFIG::TO_DIST, value); 
    break;    
  case CC_ENV_2_HPF:
    ccFlag(&m_conf->modEnvDest, &m_conf->modEnvDestNeg, TONE_CONFIG::TO_HPF, value); 
    break;    
  case CC_ENV_2_DETUNE:
    ccFlag(&m_conf->modEnvDest, &m_conf->modEnvDestNeg, TONE_CONFIG::TO_DETUNE, value); 
    break;    
  case CC_ENV_2_LFO_RATE:
    ccFlag(&m_conf->modEnvDest, &m_conf->modEnvDestNeg, TONE_CONFIG::TO_LFO_RATE, value); 
    break;    
  case CC_ENV_2_LFO_DEPTH:
    ccFlag(&m_conf->modEnvDest, &m_conf->modEnvDestNeg, TONE_CONFIG::TO_LFO_DEPTH, value); 
    break;    
  case CC_ENV_2_ARP_RATE:
    ccFlag(&m_conf->modEnvDest, &m_conf->modEnvDestNeg, TONE_CONFIG::TO_ARP_RATE, value); 
    break;    
          
  // MOD MATRIX SETTINGS FOR MOD WHEEL
  case CC_MOD_2_VOL:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_VOL, value); 
    break;    
  case CC_MOD_2_DIST:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_DIST, value); 
    break;    
  case CC_MOD_2_HPF:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_HPF, value); 
    break;    
  case CC_MOD_2_DETUNE:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_DETUNE, value); 
    break;    
  case CC_MOD_2_LFO_RATE:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_LFO_RATE, value); 
    break;    
  case CC_MOD_2_LFO_DEPTH:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_LFO_DEPTH, value); 
    break;    
  case CC_MOD_2_ARP_RATE:
    ccFlag(&m_conf->modWheelDest, &m_conf->modWheelDestNeg, TONE_CONFIG::TO_ARP_RATE, value); 
    break;        
  }
}

///////////////////////////////////////////////////////////////////////  
// HANDLE MIDI MESSAGE
void CLogicalChannel::handle(byte status, byte *params)
{
  byte cmd = (status & 0xF0);
  switch(cmd) {    
  case 0x80:
  case 0x90:
    if(cmd == 0x80 || cmd == 0x90) {
      if(cmd == 0x80 || params[1] == 0x00) {
        handleNoteOff(params[0]);
      }
      else {
        handleNoteOn(params[0], params[1]);
      }        
    }
    break;
  case 0xB0: 
    handleCC(params[0], params[1]);
    break;
  case 0xE0: 
    handlePitchBend(params[0], params[1]);
    break;
  }    
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::range(byte v) {
  for(int i=0; i<m_voiceCount; ++i) {
    m_voices[i].range(v);
  }
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::poly9(byte v) {
  for(int i=0; i<m_voiceCount; ++i) {
    m_voices[i].poly9(v);
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
    case TONE_CONFIG::DETUNE_FINE:
    case TONE_CONFIG::DETUNE_SPREAD:
      if(!i) mult = 0;
      else if(i&1) mult = (i+1)/2;
      else mult = -(i/2);
      break;
      // this mode divides voices between trigger 
      // note and trigger + interval
    case TONE_CONFIG::DETUNE_INTERVAL:
      mult = (i&1);
      break;
      // this mode stacks intervals one one top of other 
      // above played note (or below for -ve detune)
    case TONE_CONFIG::DETUNE_STACK:
      mult = i;
      break;      
      // detune is disabled
    case TONE_CONFIG::DETUNE_NONE:
    default:
      mult = 0;
      break;
    }
    m_voices[i].m_detuneFactor = mult;
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runEnvelopes() 
{
    for(int i=0; i<m_voiceCount; ++i) {       
      if(runEnvelope(&m_conf->ampEnv, &m_voices[i].m_amp)) {
        m_voices[i].reset();
//        m_voices[i].m_midi_note = 0;
      }
      runEnvelope(&m_conf->modEnv, &m_voices[i].m_mod);
    }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runLFO() 
{
    // work out if the LFO counter should be running
    byte lfoRun = 1;
    switch(m_conf->eLFOMode)
    {      
    case TONE_CONFIG::LFO_HOLD:
      lfoRun = 0;
      break;
    case TONE_CONFIG::LFO_ONE_SHOT:
      lfoRun = !(m_flags & SF_LFOCOMPLETE);
      break; 
    case TONE_CONFIG::LFO_TRIG_GATE:
    case TONE_CONFIG::LFO_GATE:   
      lfoRun = !!m_noteCount;
      break;          
    case TONE_CONFIG::LFO_UNGATE:
      lfoRun = !m_noteCount;
      break;
    }    
    if(lfoRun) {        
           
      // Modulation of the LFO rate
      float fLFOStep = m_conf->fLFOStep;
      if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_RATE) {
        fLFOStep *= m_fModWheel;
      }
      else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_LFO_RATE) {
        fLFOStep *= (1.0-m_fModWheel);
      }

      if(m_conf->flags & TONE_CONFIG::UNISON) {
        if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_RATE) {
          fLFOStep *= m_voices[0].m_mod.fValue;
        }
        else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_LFO_RATE) {
          fLFOStep *= (1.0-m_voices[0].m_mod.fValue);
        }
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
    case TONE_CONFIG::WAVE_TRI:  // TRIANGLE
      m_fLFO = m_fLFOCount;
      break;
    case TONE_CONFIG::WAVE_SQ:   // SQUARE  
      m_fLFO = (m_flags & SF_LFOSIGN)? 0.0 : 1.0;
      break;
    case TONE_CONFIG::WAVE_RAMP:    // RAMP UP
      m_fLFO = (m_flags & SF_LFOSIGN)? (m_fLFOCount/2.0) : (1.0 - (m_fLFOCount/2.0));
      break;
    case TONE_CONFIG::WAVE_REVRAMP:    // RAMP DOWN
      m_fLFO = (m_flags & SF_LFOSIGN)? (1.0 - (m_fLFOCount/2.0)) : (m_fLFOCount/2.0);
      break;
    case TONE_CONFIG::WAVE_RANDOM:
      if(m_fLFOCount == 0.0) {
        m_fLFO = random(1000) / 1000.0;
      }
      break;
    }      

    m_fLFO *= m_conf->lfoDepth/127.0;
    if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_DEPTH) {
      m_fLFO *= m_fModWheel;
    }
    else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_LFO_DEPTH) {
      m_fLFO *= (1.0 - m_fModWheel);
    }
    
    
    if(m_conf->flags & TONE_CONFIG::UNISON) {
      if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_DEPTH) {
        m_fLFO *= m_voices[0].m_mod.fValue;
      }
      else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_LFO_DEPTH) {
        m_fLFO *= (1.0 - m_voices[0].m_mod.fValue);
      }
    }

    // calculate bipolar LFO
    m_fLFOBipolar = (2.0*m_fLFO)-1.0;
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runPortamento() 
{
  // is portamento in progress? 
  if(m_portaTargetNote) {
    // calculate next note    
    float d = m_fPortamentoNote + m_fPortaStep;
    if(m_fPortamentoNote > m_portaTargetNote && d < m_portaTargetNote) {
      // reached target note
      m_fPortamentoNote = m_portaTargetNote;
      m_portaTargetNote = 0;
    }
    else if(m_fPortamentoNote < m_portaTargetNote && d > m_portaTargetNote) {
      // reached target note
      m_fPortamentoNote = m_portaTargetNote;
      m_portaTargetNote = 0;
    }
    else {
      // still portamenting..
      m_fPortamentoNote = d;
    }
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runDetune() 
{
    switch(m_conf->eDetuneMode) {
    case TONE_CONFIG::DETUNE_FINE:  // 10 cents
      m_fDetuneStep = m_conf->detuneLevel/10.0;
      break;
    case TONE_CONFIG::DETUNE_INTERVAL:  // semitone
    case TONE_CONFIG::DETUNE_SPREAD:
    case TONE_CONFIG::DETUNE_STACK:
      m_fDetuneStep = m_conf->detuneLevel;
      break;
    case TONE_CONFIG::DETUNE_NONE:
    default:
      m_fDetuneStep = 0.0;
      break;
    }
    
    if(m_conf->modEnvDest & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep *= m_fModWheel;
    } 
    else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep *= (1.0-m_fModWheel);
    }

    
    if(m_conf->flags & TONE_CONFIG::UNISON) {
      if(m_conf->modEnvDest & TONE_CONFIG::TO_DETUNE) {
        m_fDetuneStep *= m_voices[0].m_mod.fValue;
      } 
      else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_DETUNE) {
        m_fDetuneStep *= (1.0-m_voices[0].m_mod.fValue);
      }
    }
    
    if(m_conf->lfoDest & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep *= m_fLFO;
    } 
    else if(m_conf->lfoDestNeg & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep *= (1.0-m_fLFO);
    }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runArpeggiator() 
{
    if((m_conf->flags & TONE_CONFIG::ARPEGGIATE) && (m_noteCount > 0)) {

      float arpPeriod = m_conf->arpPeriod;
      if(m_conf->modEnvDest & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= (1.0-m_voices[0].m_mod.fValue); // NB - mod is of RATE not period
      } 
      else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= m_voices[0].m_mod.fValue;
      }

      if(m_conf->lfoDest & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= (1.0-m_fLFO);
      } 
      else if(m_conf->lfoDestNeg & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= m_fLFO;
      }
      
      if(--m_arpCounter <= 0)
      {          
        if(m_arpIndex >= m_noteCount) { // reached the last note?            
          if(m_noteCount > m_conf->arpCount) { // need to arp just recent notes?
            m_arpIndex = m_noteCount - m_conf->arpCount;
          } 
          else {
            m_arpIndex = 0;
          }            
        }      
        trig(m_notes[m_arpIndex].note,  (m_conf->flags & TONE_CONFIG::USE_VELOCITY)? m_notes[m_arpIndex].velocity : 127, !!(m_conf->flags & TONE_CONFIG::ARP2ENV));      
        ++m_arpIndex;
        m_arpCounter = arpPeriod;
      }
      else if(m_arpCounter > m_conf->arpPeriod) {
        // in case CC is being swept down from a long value
        m_arpCounter = arpPeriod;
      }
    }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::update(byte ticks)  
{
    switch(ticks & 0x7) {
      case 0: runEnvelopes(); 
        break;
      case 1: runLFO();
        break;
      case 2: runPortamento();
        break;
      case 3: runDetune();
        break;
      case 4: runArpeggiator();
        break;
    }
}


///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
//  indicateTest(1);

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
//
// HELPER FUNCTIONS
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
inline byte isSingleNoteChan(TONE_CONFIG *conf) {
  return ( (conf->flags & TONE_CONFIG::MONO) || conf->eUnisonMode != TONE_CONFIG::UNISON_OFF);
}

///////////////////////////////////////////////////////////////////////
inline unsigned int envSlope(char v, byte allowZeroSlope) {
  if(v == 127 && allowZeroSlope) return 0;  
  return 65535/(v+1);
}

///////////////////////////////////////////////////////////////////////
inline void trigEnvelope(ENVELOPE *env, ENVELOPE_STATE *envState) {
  envState->ePhase = ENVELOPE_STATE::ATTACK;    
  if(env->attackSlope == 65535) {
    envState->fValue = 1.0; // no attack - full volume immediately!
  }
  else {
    envState->fValue = 0.0; // start of attack
  }
}

///////////////////////////////////////////////////////////////////////
inline void untrigEnvelope(ENVELOPE *env, ENVELOPE_STATE *envState) {
  switch(env->mode) {            
    case ENVELOPE::ATT_DEC:
    case ENVELOPE::ATT_REL:
    case ENVELOPE::ATT_RPT_REL:
      switch(envState->ePhase) {
        case ENVELOPE_STATE::ATTACK:
        case ENVELOPE_STATE::SUSTAIN:
          envState->ePhase = ENVELOPE_STATE::RELEASE;
          if(env->releaseSlope == 65535) {
            envState->fValue = 0.0; // no release - kill volume immediately
          }
          break;
      }  
      break;        
    case ENVELOPE::ATT_DEC_RPT:
      // kill it dead
      envState->fValue = 0.0;
      envState->ePhase = ENVELOPE_STATE::NONE;
      break;
    case ENVELOPE::LOOP:
      // keeps right on going!
      break;
  }  
}

///////////////////////////////////////////////////////////////////////
// result is 1 if envelope has finished
inline byte runEnvelope(ENVELOPE *env, ENVELOPE_STATE *envState) {  
  
  switch(envState->ePhase) {               
    case ENVELOPE_STATE::ATTACK:
        envState->fValue += env->attackSlope/65535.0;
        if(envState->fValue >= 1.0) {
          envState->fValue = 1.0;
          // reached end of attack phase... what happens next depends on mode
          switch(env->mode) {            
            case ENVELOPE::ATT_DEC:
            case ENVELOPE::ATT_DEC_RPT:
            case ENVELOPE::LOOP:
              envState->ePhase = ENVELOPE_STATE::RELEASE;
              break;
            case ENVELOPE::ATT_RPT_REL:
              envState->fValue = 0.0;
              envState->ePhase = ENVELOPE_STATE::ATTACK;
              break;
            case ENVELOPE::ATT_REL:
            default:
              envState->ePhase = ENVELOPE_STATE::SUSTAIN;
              break;
          }
        }
        break;
    case ENVELOPE_STATE::RELEASE:
        envState->fValue -= env->releaseSlope/65535.0;
        if(envState->fValue <= 0.0) {
          envState->fValue = 0.0;
          // reached end of release phase... what happens next depends on mode
          switch(env->mode) {                        
            case ENVELOPE::LOOP:
            case ENVELOPE::ATT_DEC_RPT:
              envState->ePhase = ENVELOPE_STATE::ATTACK;
              break;
            case ENVELOPE::ATT_RPT_REL:
            case ENVELOPE::ATT_DEC:
            case ENVELOPE::ATT_REL:
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
//
// PRIVATE MEMBERS
//
///////////////////////////////////////////////////////////////////////

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
// Called when a new note is triggered on the channel
void CLogicalChannel::trig(byte note, byte velocity, byte trigEnv) 
{    
  CLogicalVoice *voice = NULL;
  
  // Check if the channel LFO needs to be restarted 
  // on each note trigger
  switch(m_conf->eLFOMode)
  {
  case TONE_CONFIG::LFO_TRIG_FREE:
  case TONE_CONFIG::LFO_TRIG_GATE:
  case TONE_CONFIG::LFO_ONE_SHOT:
    // Yes - so restart the LFO cycle
    m_fLFOPhase = 0.0;
    m_flags |= SF_LFOSIGN;
    m_flags &= ~SF_LFOCOMPLETE;
    break;
  case TONE_CONFIG::LFO_UNGATE:
  case TONE_CONFIG::LFO_GATE:   
  case TONE_CONFIG::LFO_FREE:      
  default:
    break;
  }

  //////////////////////////////////////////////////////////////////
  // CHANNEL CONFIGURED IN UNISON OR MONO MODE
  if((m_conf->eUnisonMode != TONE_CONFIG::UNISON_OFF) || 
      ( m_conf->flags & TONE_CONFIG::MONO)) {
    // check if a nonzero portamento setting is defined
    if(m_conf->portaTime) {
      
      // store trigggered note as the target of the portamento engine
      m_portaTargetNote = note;
      
      // is there a previous note to use as start point?
      if(m_fPortamentoNote > 0 && note != m_fPortamentoNote) {  
        
        // calculate step rate based on distance between the old and new note
        // and the portamento time setting        
        m_fPortaStep = 2.0 * (note - m_fPortamentoNote) / (float)m_conf->portaTime;
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
    
    for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
      voice = &Voice[i];
      voice->m_midi_note = note;
      voice->m_midi_vel = velocity;
      if(trigEnv) {
        trigEnvelope(&m_conf->ampEnv, &voice->m_amp);
        trigEnvelope(&m_conf->modEnv, &voice->m_mod);
      }
      if(m_conf->flags & TONE_CONFIG::MONO) {
        break;
      }
    }
  }
  //////////////////////////////////////////////////////////////////
  // POLYPHONIC CHANNEL
  else
  {    
    // check if the note is already assigned to a channel...
    // if it is, then we will retrigger it
    for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
      if(Voice[i].m_midi_note == note) {
        voice = &Voice[i];
        break;
      }
    }
    if(!voice) {
      // Otherwise do we have a free channel?
      for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
        if(Voice[i].m_amp.ePhase == ENVELOPE_STATE::NONE) {
          voice = &Voice[i];
          break;
        }
      }
    }
    if(!voice) {
      // or failing that, one that is in release
      for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
        if(Voice[i].m_amp.ePhase == ENVELOPE_STATE::RELEASE) {
          voice = &Voice[i];
          break;
        }
      }
    }
    
    // Did we get a voice to assign the note to?
    if(voice) {
      
      // assign the note to the voice
      voice->m_midi_note = note;
      voice->m_midi_vel = velocity;
      
      // retrigger envelopes if needed
      if(trigEnv) {
        trigEnvelope(&m_conf->ampEnv, &voice->m_amp);
        trigEnvelope(&m_conf->modEnv, &voice->m_mod);
      }
    }
  }
}  

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::untrig(byte note) {
    
  for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
    if(Voice[i].m_midi_note == note) {      
      untrigEnvelope(&m_conf->ampEnv, &Voice[i].m_amp);
      untrigEnvelope(&m_conf->modEnv, &Voice[i].m_mod);
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
  // Ensure the note is in our valid range
  if(note < m_fromNote || note > m_toNote) 
    return;
    
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
  int voiceCount = m_voiceEnd - m_voiceBegin;
  if(!isSingleNoteChan(m_conf) && (m_noteCount > voiceCount)) {    
    // more notes than voices, so mute the oldest note
    int nextMute = m_noteCount-voiceCount-1;
    untrig(m_notes[nextMute].note);
  }
  // trigger the newest note, triggering the amp envelope
  trig(note, (m_conf->flags & TONE_CONFIG::USE_VELOCITY)? velocity:127, true);
}

///////////////////////////////////////////////////////////////////////
// HANDLE NOTE OFF MESSAGE
void CLogicalChannel::handleNoteOff(byte note)
{
  untrig(note);
  if(deleteNote(note)) {

    int voiceCount = isSingleNoteChan(m_conf)? 1:(m_voiceEnd - m_voiceBegin);
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
  m_fPitchBend = (float)((((int)hi)<<7 | lo) - 0x2000) / 0x2000;
  m_fPitchBend *= m_conf->pitchBendRange;
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
          mode = CPokey::MODE_8BITHPF;
          break;        
        case TONE_CONFIG::POKEY_16:
          mode = CPokey::MODE_16BIT;
          break;        
        case TONE_CONFIG::POKEY_8:  
        default:
          mode = CPokey::MODE_8BIT;
          break;
      }
      if(mode != m_conf->ePokeyMode) {
        m_conf->ePokeyMode = mode;
//        m_flags |= SF_RECONFIG;
      }
    }
    break;
  case CC_POKEYDUAL:
    {
      byte flags = m_conf->flags;
      ccFlag(&flags, TONE_CONFIG::POKEY_DUAL, value); 
      if(flags != m_conf->flags) {
//        m_flags |= SF_RECONFIG;
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
    div8_high(!!(m_conf->flags & TONE_CONFIG::POKEY_HIHZ)); 
    break;    
  case CC_MIDIVEL: 
    ccFlag(&m_conf->flags, TONE_CONFIG::USE_VELOCITY, value); 
    break;
//  case CC_UNISON: 
//    ccFlag(&m_conf->flags, TONE_CONFIG::UNISON, value); 
//    break;
  case CC_MONO: 
    ccFlag(&m_conf->flags, TONE_CONFIG::MONO, 127); 
    break;
  case CC_POLY: 
    ccFlag(&m_conf->flags, TONE_CONFIG::MONO, 0); 
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
    m_conf->arpRate = value;
    break;
  case CC_ARPCOUNT: 
    m_conf->arpCount = 1 + value/16; 
    break;
  case CC_LFORATE: 
    m_conf->lfoRate = value; 
    break;
  case CC_AENVATTACK: 
    m_conf->ampEnv.attackSlope = envSlope(value,0);
    break;
  case CC_AENVRELEASE: 
    m_conf->ampEnv.releaseSlope = envSlope(value,1);
    break;
  case CC_AENVMODE:
    m_conf->ampEnv.mode = ccMapValue(value, ENVELOPE::MAX_MODE); 
    break;
  case CC_MENVATTACK: 
    m_conf->modEnv.attackSlope = envSlope(value,0);
    break;
  case CC_MENVRELEASE: 
    m_conf->modEnv.releaseSlope = envSlope(value,1);
    break;
  case CC_MENVMODE: 
    m_conf->modEnv.mode = ccMapValue(value, ENVELOPE::MAX_MODE); 
    break;    
  case CC_PBRANGE: 
    m_conf->pitchBendRange = 12.0 * (value/127.0); 
    break;
  case CC_HPF: 
    m_conf->hpf = value;
    break;
  case CC_DIST: 
    m_conf->dist = value; 
    break;    
  case CC_DETUNELEVEL: 
    m_conf->detuneLevel = value; 
    break;
  case CC_UNISONTYPE: 
    m_conf->eUnisonMode = ccMapValue(value, TONE_CONFIG::UNISON_MAX); 
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
    m_conf->lfo2Pitch = value; 
    break;    
  case CC_LFO_2_VOL:
    m_conf->lfo2Vol = value; 
    break;
  case CC_LFO_2_MATRIX:
    m_conf->lfoDepth = value; 
    break;        
  case CC_LFO_2_DIST:
    ccFlag(&m_conf->lfoDest, TONE_CONFIG::TO_DIST, value); 
    break;    
  case CC_LFO_2_HPF:
    ccFlag(&m_conf->lfoDest, TONE_CONFIG::TO_HPF, value); 
    break;    
  case CC_LFO_2_DETUNE:
    ccFlag(&m_conf->lfoDest, TONE_CONFIG::TO_DETUNE, value); 
    break;    
  case CC_LFO_2_ARP_RATE:
    ccFlag(&m_conf->lfoDest, TONE_CONFIG::TO_ARP_RATE, value); 
    break;    

  // MOD MATRIX SETTINGS FOR MOD ENVELOPE          
  case CC_ENV_2_PITCH:
    m_conf->modEnv2Pitch = value-64; 
    break;    
  case CC_ENV_2_MATRIX:
    m_conf->modEnvDepth = value-64; 
    break;    
  case CC_ENV_2_DISTORTION:
    ccFlag(&m_conf->modEnvDest, TONE_CONFIG::TO_DIST, value); 
    break;    
  case CC_ENV_2_HPF:
    ccFlag(&m_conf->modEnvDest, TONE_CONFIG::TO_HPF, value); 
    break;    
  case CC_ENV_2_DETUNE:
    ccFlag(&m_conf->modEnvDest, TONE_CONFIG::TO_DETUNE, value); 
    break;    
  case CC_ENV_2_LFO_RATE:
    ccFlag(&m_conf->modEnvDest, TONE_CONFIG::TO_LFO_RATE, value); 
    break;    
  case CC_ENV_2_LFO_DEPTH:
    ccFlag(&m_conf->modEnvDest, TONE_CONFIG::TO_LFO_DEPTH, value); 
    break;    
  case CC_ENV_2_ARP_RATE:
    ccFlag(&m_conf->modEnvDest, TONE_CONFIG::TO_ARP_RATE, value); 
    break;    
          
  // MOD MATRIX SETTINGS FOR MOD WHEEL
  case CC_MOD_2_VOL:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_VOL, value); 
    break;    
  case CC_MOD_2_DIST:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_DIST, value); 
    break;    
  case CC_MOD_2_HPF:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_HPF, value); 
    break;    
  case CC_MOD_2_DETUNE:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_DETUNE, value); 
    break;    
  case CC_MOD_2_LFO_RATE:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_LFO_RATE, value); 
    break;    
  case CC_MOD_2_LFO_DEPTH:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_LFO_DEPTH, value); 
    break;    
  case CC_MOD_2_ARP_RATE:
    ccFlag(&m_conf->modWheelDest, TONE_CONFIG::TO_ARP_RATE, value); 
    break;        
  case CC_ALL_SOUND_OFF:
  case CC_ALL_NOTES_OFF:
    quiet();
    break;
  case CC_RESET_CTRL:
    //TODO
    break;
  case CC_FROM_NOTE:
    m_fromNote = value;
    if(m_toNote < m_fromNote) {
      m_fromNote = m_toNote;
      m_toNote = value;
    }
    break;
  case CC_TO_NOTE:    
    m_toNote = value;
    if(m_toNote < m_fromNote) {
      m_toNote = m_fromNote;
      m_fromNote = value;
    }
    break;
  }
}

///////////////////////////////////////////////////////////////////////
// RECALCULATE VOICE DETUNE FACTORS
// To "spread" the detuning of unison voices, each voice in the channel
// has a detune factor that the detune amount is multiplied by. This 
// method calculates the detune factors for each voice based on the 
// detune mode
void CLogicalChannel::recalc_detune()
{
  // lookup table of factors for "spread" mode
  const char spread[] = {0, -1, +1, -2, +2, -3, +3, -4};
  
  int idx = 0;
  for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
    switch(m_conf->eUnisonMode) {
      case TONE_CONFIG::UNISON_SPREAD:
        Voice[i].m_detuneFactor = spread[idx++]; // use spread
        break;
      case TONE_CONFIG::UNISON_INTERVALUP:      
        Voice[i].m_detuneFactor = ((idx++)&1)? 1: 0; // alternate 0 and 1
        break;
      case TONE_CONFIG::UNISON_INTERVALDOWN:      
        Voice[i].m_detuneFactor = ((idx++)&1)? -1: 0; // alternate 0 and -1
        break;
      case TONE_CONFIG::UNISON_STACKUP:
        Voice[i].m_detuneFactor = idx++; // increment 1,2,3...
        break;
      case TONE_CONFIG::UNISON_STACKDOWN:
        Voice[i].m_detuneFactor = -(idx++); // increment -1,-2,-3...
        break;
      default:
      case TONE_CONFIG::UNISON_OFF:
        Voice[i].m_detuneFactor = 0; // no detune
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runEnvelopes() 
{
    for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {       
      if(runEnvelope(&m_conf->ampEnv, &Voice[i].m_amp)) {
        Voice[i].quiet();
      }
      runEnvelope(&m_conf->modEnv, &Voice[i].m_mod);
    }
}

///////////////////////////////////////////////////////////////////////
// RUN THE LFO
// The LFO member, m_fLFO, has a value -1.0 .. +1.0 based on the 
// selected wave form. All wave types are derived from an internal
// triangle wave for which one complete cycle runs 0 .. +1 .. 0 .. -1 .. 
void CLogicalChannel::runLFO() 
{
    // The following block gates the LFO based on the 
    // LFO run mode
    byte lfoRun = 1;
    switch(m_conf->eLFOMode)
    {      
    case TONE_CONFIG::LFO_HOLD:
      lfoRun = 0;  // LFO held - not running
      break;
    case TONE_CONFIG::LFO_ONE_SHOT:
      lfoRun = !(m_flags & SF_LFOCOMPLETE); // LFO in one shot mode
      break; 
    case TONE_CONFIG::LFO_TRIG_GATE:
    case TONE_CONFIG::LFO_GATE:   
      if(m_noteCount) {
        lfoRun = 1; // LFO runs only when notes are held
      } else {
        m_fLFOPhase = 0.0; // reset the LFO phase
      }      
      break;          
    case TONE_CONFIG::LFO_UNGATE:
      if(!m_noteCount) {
        lfoRun = 1;  // LFO runs only when notes are not held
      }
      else {
        m_fLFOPhase = 0.0; // reset the LFO phase
      }
      break;
    }    

    // flags a zero crossing
    byte zeroCrossing = 0;
    
    // is the LFO running?
    if(lfoRun) {        
                 
      // Get floating point LFO rate
      float fLFORate = (1+m_conf->lfoRate)/127.0;
      
      /*
      if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_RATE) {
        fLFOStep *= m_fModWheel;
      }

      if(isSingleNoteChan(m_conf)) {
        if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_RATE) {
          fLFOStep *= Voice[m_voiceBegin].m_mod.fValue;
        }
      }
      */

      // Update the phase of the triangle wave counter
      if(m_flags & SF_LFOSIGN) {
        // wave is rising
        float f = m_fLFOPhase + fLFORate;
        if(m_fLFOPhase < 0 && f > 0) {
          zeroCrossing = 1;
        }
        if(f >= 1.0) {
          // start to fall
          m_fLFOPhase = 1.0;
          m_flags &= ~SF_LFOSIGN;
        } else {
          m_fLFOPhase = f;
        }
      }
      else {
        // wave is falling
        float f = m_fLFOPhase - fLFORate;
        if(m_fLFOPhase > 0 && f < 0) {
          zeroCrossing = 1;
          // zero crossing on falling wave marks end of cycle
          m_flags |= SF_LFOCOMPLETE; 
        }
        if(f  <= -1.0) {
          // start to rise
          m_fLFOPhase = -1.0;
          m_flags |= SF_LFOSIGN;
        }
        else {
          m_fLFOPhase = f;
        }
      }
    }

    // The following block converts the triangle wave phase
    // into the appropriate LFO wave form    
    switch(m_conf->eLFOWave) 
    {
    // TRIANGLE WAVE LFO
    // simples!
    case TONE_CONFIG::WAVE_TRI:  
      m_fLFO = m_fLFOPhase; 
      break;

    // SQUARE WAVE LFO
    // take the sign of the triangle value
    case TONE_CONFIG::WAVE_SQ:  
      m_fLFO = (m_fLFOPhase > 0)? 1.0 : -1.0;
      break;

    // RAMP LFO
    // transform triangle based on quadrant of cycle
    // and then negate for reverse ramp
    case TONE_CONFIG::WAVE_RAMP:    
    case TONE_CONFIG::WAVE_REVRAMP: 
      if(m_flags & SF_LFOSIGN) { // rising
        m_fLFO = m_fLFOPhase/2.0;
      }
      else { // falling wave
        if(m_fLFOPhase >= 0) {  // positive value 
          m_fLFO = 1.0 - m_fLFOPhase/2.0;
        }
        else {  // negative value
          m_fLFO = -1.0 - m_fLFOPhase/2.0;
        }
      }
      if(m_conf->eLFOWave == TONE_CONFIG::WAVE_REVRAMP) {
        m_fLFO = -m_fLFO;
      }
      break;

    // RANDOM LFO
    // create a pulse wave where both positive and 
    // negative amplitudes are random (but have the
    // correct sign)
    case TONE_CONFIG::WAVE_RANDOM:
      if(zeroCrossing) {
        m_fLFO = random(1000)/ 1000.0;
        if(!(m_flags & SF_LFOSIGN)) {
          m_fLFO = - m_fLFO;
        }
      }
      break;    
    }      
    
    // calculate the positive-only LFO
    m_fLFOPositive = 0.5 + m_fLFO/2.0;

    /*
    m_fLFO *= m_conf->lfoDepth/127.0;
    if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_DEPTH) {
      m_fLFO *= m_fModWheel;
    }
    */
    
    /*
    if(isSingleNoteChan(m_conf)) {
      if(m_conf->modEnvDest & TONE_CONFIG::TO_LFO_DEPTH) {
        m_fLFO *= Voice[m_voiceBegin].m_mod.fValue;
      }
    }
    */
    
    
}

///////////////////////////////////////////////////////////////////////
// RUN PORTAMENTO STATE MACHINE
void CLogicalChannel::runPortamento() 
{
  // is portamento in progress? 
  if(m_portaTargetNote) {
    
    // calculate next note    
    m_fPortamentoNote += m_fPortaStep;
    
    // gliding up?
    if(m_fPortaStep > 0.001) {
      
      // reached target note?
      if(m_fPortamentoNote >= m_portaTargetNote) {
        
        // stop portamento-ing!
        m_fPortamentoNote = m_portaTargetNote;
        m_portaTargetNote = 0;
      }
    }
    // gliding down?
    else if(m_fPortaStep < -0.001) {
      
      // reached target note?
      if(m_fPortamentoNote <= m_portaTargetNote) {
        
        // stop
        m_fPortamentoNote = m_portaTargetNote;
        m_portaTargetNote = 0;
      }
    }
    else {
      
        // hmm..
        m_fPortamentoNote = m_portaTargetNote;
        m_portaTargetNote = 0;
    }
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::runDetune() 
{
    if(!isSingleNoteChan(m_conf)) {
      m_fDetuneStep = 0.0;
      return;
    }
  
    // Mod wheel will override the detune step
    if(m_conf->modWheelDest & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep = m_fModWheel * 12.0;
    } 
    else {
      m_fDetuneStep = m_conf->detuneLevel/10.0;
    }

/*    
    if(m_conf->flags & TONE_CONFIG::UNISON) {
      if(m_conf->modEnvDest & TONE_CONFIG::TO_DETUNE) {
        m_fDetuneStep *= Voice[m_voiceBegin].m_mod.fValue;
      } 
      else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_DETUNE) {
        m_fDetuneStep *= (1.0-Voice[m_voiceBegin].m_mod.fValue);
      }
    }
    
    if(m_conf->lfoDest & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep *= m_fLFO;
    } 
    else if(m_conf->lfoDestNeg & TONE_CONFIG::TO_DETUNE) {
      m_fDetuneStep *= (1.0-m_fLFO);
    }
    */
}

///////////////////////////////////////////////////////////////////////
// RUN THE ARPEGGIATOR STATE MACHINE
void CLogicalChannel::runArpeggiator() 
{
    // Arpeggiator can only run on single note channel (mono or unison) and only
    // if arpeggiation is enabled!
    if((m_conf->flags & TONE_CONFIG::ARPEGGIATE) && isSingleNoteChan(m_conf) && (m_noteCount > 0)) {

      // get arp rate (could be from mod wheel)
      float arpRate;      
      if(m_conf->modWheelDest & TONE_CONFIG::TO_ARP_RATE) {
        arpRate = m_fModWheel; 
      } 
      else {
         arpRate = m_conf->arpRate/127.0;
      }
      
/*      
      if(m_conf->modEnvDest & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= (1.0-Voice[m_voiceBegin].m_mod.fValue); // NB - mod is of RATE not period
      } 
      else if(m_conf->modEnvDestNeg & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= Voice[m_voiceBegin].m_mod.fValue;
      }

      if(m_conf->lfoDest & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= (1.0-m_fLFO);
      } 
      else if(m_conf->lfoDestNeg & TONE_CONFIG::TO_ARP_RATE) {
        arpPeriod *= m_fLFO;
      }
*/      
      // convert arp rate into arp period
      int arpPeriod = 100/(1+arpRate) - 50;
      
      // check if next note is due
      if(--m_arpCounter <= 0)
      {          
        // determine the next note
        if(m_arpIndex >= m_noteCount) { // reached the last note?            
          if(m_noteCount > m_conf->arpCount) { // need to arp just recent notes?
            m_arpIndex = m_noteCount - m_conf->arpCount;
          } 
          else {
            m_arpIndex = 0;
          }            
        }      
        
        // trigger the bext note
        trig(
          m_notes[m_arpIndex].note,  
          (m_conf->flags & TONE_CONFIG::USE_VELOCITY)? m_notes[m_arpIndex].velocity : 127, !!(m_conf->flags & TONE_CONFIG::ARP2ENV)
        );      
        ++m_arpIndex;
        m_arpCounter = arpPeriod;
      }
      else if(m_arpCounter > arpPeriod) {
        // in case CC is being swept down from a long value
        m_arpCounter = arpPeriod;
      }
    }
}


///////////////////////////////////////////////////////////////////////
//
// PUBLIC MEMBERS
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CLogicalChannel::CLogicalChannel() 
{
  m_conf = NULL;
  m_voiceBegin = 0;
  m_voiceEnd = 0;
  m_fromNote = 0;
  m_toNote = 127;
  m_midiChannel = 0;
}

///////////////////////////////////////////////////////////////////////  
void CLogicalChannel::assign(byte voiceBegin, byte voiceEnd, TONE_CONFIG *conf)
{
  m_voiceBegin = voiceBegin;
  m_voiceEnd = voiceEnd;
  m_conf = conf;
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::start() {

  m_flags = 0;
  m_fPitchBend = 0;
  m_fModWheel = 0;  
  m_fLFO = 0.0;
  m_fPortamentoNote = 0.0;
  m_fDetuneStep = 0.0;  
  m_arpCounter = 0;
  m_arpIndex = 0;
  m_fLFOPhase = 0;
  m_portaTargetNote = 0;
  m_fPortaStep = 0;   
  
  // ensure no sound
  quiet();
  
  // initialise the high divider flags for each voice
  for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
    Voice[i].div8_high(m_conf->flags & TONE_CONFIG::POKEY_HIHZ);     
  }
  
  // initial calculation of detune spread
  recalc_detune();
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::quiet() {
  // clear the note stack
  m_noteCount = 0;
  
  // silence the voices
  for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
    Voice[i].quiet();
  }   
}


///////////////////////////////////////////////////////////////////////  
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
void CLogicalChannel::div8_high(byte v) {
  for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
    Voice[i].div8_high(v);
  }
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::dist_poly9(byte v) {
  for(int i=m_voiceBegin; i<m_voiceEnd; ++i) {
    Voice[i].dist_poly9(v);
  }
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::update(byte ticks)  
{
//  indicateTest(m_noteCount > 1);
  
    // This method is called every 1 millisecond
    // The channel state machine updates run on a 16ms repeating 
    // cycle. This switch decides the order (and number of times)
    // each of the channel state machines gets invoked on each
    // cycle
    switch(ticks & 0xF) {
      // ENVELOPE - once every 16ms
      case 0: 
        runEnvelopes(); 
        break;
      case 1: 
        runLFO();
        break;
      case 2: 
        runPortamento();
        break;
      case 3: 
        runDetune();
        break;
      case 4: 
      case 12: // arp clock runs twice per cycle
        runArpeggiator();
        break;
    }
}


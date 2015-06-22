//TODO inhibit portamento on poly channel

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
  m_vel = 0;
  m_envPhase = CLogicalVoice::ENV_NONE;
  m_detune = 0;
}
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::assign(CPokeyChannel *pch) 
{
  m_pch = pch;
} 
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::update(CLogicalChannel *lch) 
{  
  if(!m_pch)
    return;
  float n;
  if(lch->m_portaLevel) {
    n = lch->m_portamento;
  }
  else {
    n = m_note;
  }
  if(n>1) {    
    float note = n + m_detune + lch->m_bend + lch->m_vibrato;
    float freq = 440.0 * pow(2.0,((note-57.0)/12.0));
    m_pch->pitch(freq);

    float vol = lch->m_tremelo * 255.0 * (m_envLevel/65535.0);
    m_pch->vol(vol);
    
//    m_pch->dist(CPokeyChannel::DIST_5);
//    DIST_5     = 0b01100000,
//    DIST_5_4   = 0b01000000,
//    DIST_5_17  = 0b00000000,
//    DIST_17    = 0b10000000,
  }
  else {
    m_pch->vol(0);
  }
}

///////////////////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CLogicalChannel::CLogicalChannel() 
{
  m_flags = FLAG_UNISON|FLAG_FULLVELOCITY;
  m_voices = NULL;
  m_midiChannel = OMNI;
  m_noteCount = 0;
  m_voiceCount = 0;
  m_bendRange = 3;
  m_bend = 0;
  m_attack = 65535;
  m_release = 60;
  m_tremStep = 300;
  m_tremCounter = 0;
  m_tremelo = 1.0;
  m_tremLevel = 0;
  
  m_vibStep = 300;
  m_vibCounter = 0;
  m_vibrato = 0.0;
  m_vibLevel = 0;
   
  m_portaLevel = 2;
  m_portaTarget = 0;
  m_portamento = 0.0;
  m_portaStep = 0.0;

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
  if(cmd == 0x80 || cmd == 0x90) {
    if((status & 0x0F) == m_midiChannel || OMNI == m_midiChannel) {
      if(cmd == 0x80 || params[1] == 0x00) {
        handleNoteOff(params[0]);
      }
      else {
        handleNoteOn(params[0], params[1]);
      }        
    }
  }
  else if(cmd == 0xE0) {
    if((status & 0x0F) == m_midiChannel || OMNI == m_midiChannel) {
      handlePitchBend(params[0], params[1]);
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
  trig(
  note, 
  velocity
    );
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
      trig(
        m_notes[m_noteCount-voiceCount].note, 
        m_notes[m_noteCount-voiceCount].velocity
       );
    }
  }
}
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::handlePitchBend(byte lo, byte hi)
{
  m_bend = ((((int)hi)<<7 | lo) - 8192);
  m_bend *= m_bendRange;
  m_bend /= 16384;
  for(int i=0; i<m_voiceCount; ++i) {
    if(m_voices[i].m_note) {
      m_voices[i].update(this);    
    }
  }    
}  
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::trig(byte note, byte velocity) {    

  CLogicalVoice *voice = NULL;
  if(m_portaLevel) {
    m_portaTarget = note;
    if(m_portamento>0) {  
      m_portaStep = (note - m_portamento) / (m_portaLevel * 10.0);
      note = m_portamento;
    }
    else {
      m_portamento = note;
      m_portaStep = 127;
    }
  }
  
  if(m_flags & FLAG_UNISON)
  {
      float n = note;
      for(int i=0; i<m_voiceCount; ++i) {
        voice = &m_voices[i];
        voice->m_note = n;
        voice->m_vel = velocity;
        voice->m_envPhase = CLogicalVoice::ENV_ATTACK;    
        voice->m_envLevel = 0;
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
        if(m_voices[i].m_envPhase == CLogicalVoice::ENV_NONE) {
          voice = &m_voices[i];
          break;
        }
      }
    }
    if(!voice) {
      // or failing that, one that is in release
      for(int i=0; i<m_voiceCount; ++i) {
        if(m_voices[i].m_envPhase == CLogicalVoice::ENV_RELEASE) {
          voice = &m_voices[i];
          break;
        }
      }
    }
    if(voice) {
      voice->m_note = note;
      voice->m_vel = velocity;
      voice->m_envPhase = CLogicalVoice::ENV_ATTACK;    
      voice->m_envLevel = 0;
    }
  }
}  
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::untrig(byte note) {
  if(m_flags & FLAG_UNISON)
  {
    for(int i=0; i<m_voiceCount; ++i) {
      m_voices[i].m_envPhase = CLogicalVoice::ENV_RELEASE;
    }
  }
  else
  {  
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_note == note) {
        m_voices[i].m_envPhase = CLogicalVoice::ENV_RELEASE;
        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////
// TICK
// called once per millisecond... manages tremelo, vibrato, A/R envelope
void CLogicalChannel::tick() 
{
  long l;
  
  // Run Tremelo
  if(m_tremLevel) {
    l = (long)m_tremCounter + (long)m_tremStep;
    if(l>65535) {
      m_tremStep = -m_tremStep;
      m_tremCounter = 65535;
    } else if(l<0) {
      m_tremStep = -m_tremStep;
      m_tremCounter = 0;
    } else {
      m_tremCounter = l;
    }    
    m_tremelo = ((float)m_tremLevel * (float)m_tremCounter)/(65535.0 * 255.0);
  }
  else {
    m_tremelo = 1.0;
  }

  // Run Vibrato
  if(m_vibLevel) {
    l = (long)m_vibCounter + (long)m_vibStep;
    if(l>65535) {
      m_vibStep = -m_vibStep;
      m_vibCounter = 65535;
    } else if(l<0) {
      m_vibStep = -m_vibStep;
      m_vibCounter = 0;
    } else {
      m_vibCounter = l;
    }    
    m_vibrato = 12.0 * ((float)m_vibLevel * (float)(m_vibCounter - 32768))/(65535.0 * 255.0);
  }
  else {
    m_vibrato = 0.0;
  }
  
  // Run Portamento
  if(m_portaLevel && m_portaTarget) {
    float d = m_portamento + m_portaStep;
    if(m_portamento > m_portaTarget && d < m_portaTarget) {
      m_portamento = m_portaTarget;
      m_portaTarget = 0;
    }
    else if(m_portamento < m_portaTarget && d > m_portaTarget) {
      m_portamento = m_portaTarget;
      m_portaTarget = 0;
    }
    else {
      m_portamento = d;
    }
  }
  
  // Run Envelopes and update channels
  for(int i=0; i<m_voiceCount; ++i) {
    switch(m_voices[i].m_envPhase) {      
      // ATTACK!!
      case CLogicalVoice::ENV_ATTACK:
        l = (long)m_voices[i].m_envLevel + (long)m_attack;
        if(l>=65535) {
          m_voices[i].m_envPhase = CLogicalVoice::ENV_SUSTAIN;
          m_voices[i].m_envLevel = 65535;
        } 
        else {
          m_voices[i].m_envLevel = l;
        }        
        break;
      // RELEASE
      case CLogicalVoice::ENV_RELEASE:
        l = (long)m_voices[i].m_envLevel - (long)m_release;
        if(l<=0) {
          m_voices[i].m_envPhase = CLogicalVoice::ENV_NONE;
          m_voices[i].m_envLevel = 0;
        }
        else {
          m_voices[i].m_envLevel = l;
        }
        break;    
    }
    m_voices[i].update(this);
  }  
}

///////////////////////////////////////////////////////////////////////
void CLogicalChannel::detune(float detune)
{
  float d = 0;
  for(int i=0; i<m_voiceCount; ++i) {
      m_voices[i].m_detune = d;
      // Spread the detuned voices so that assigned notes are
      // n, (n+d), (n-d), (n+2d), (n-2d)... 
      if(i&1) {
        d -= ((i+1)*detune);
      }
      else {
        d   += ((i+1)*detune);
      }
  }  
}



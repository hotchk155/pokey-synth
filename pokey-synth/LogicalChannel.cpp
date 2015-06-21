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
  m_note = 0;
  m_vel = 0;
}
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::assign(CPokeyChannel *pch) 
{
  m_pch = pch;
} 
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::trig(byte note, byte vel, CLogicalChannel *lch)
{
  m_note = note;
  m_vel = vel;
  update(lch);
}
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::untrig(CLogicalChannel *lch)
{
  m_note = 0;
  m_vel = 0;
  update(lch);
}
///////////////////////////////////////////////////////////////////////
void CLogicalVoice::update(CLogicalChannel *lch) 
{
  if(!m_pch)
    return;
  if(m_note) {
    float note = m_note + lch->m_bend;
    float freq = 440.0 * pow(2.0,((note-57.0)/12.0));
    m_pch->pitch(freq);
  }
  m_pch->vol(m_vel);
}

///////////////////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CLogicalChannel::CLogicalChannel() 
{
  m_flags = 0;//FLAG_FULLVELOCITY;
  m_voices = NULL;
  m_midiChannel = OMNI;
  m_noteCount = 0;
  m_voiceCount = 0;
  m_bendRange = 3;
  m_bend = 0;
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
  } else {
    ++m_noteCount;
  }
  // place the new note at the top of the stack
  m_notes[m_noteCount-1].note = note;
  m_notes[m_noteCount-1].velocity = velocity;    
  if(m_noteCount > m_voiceCount) {    
      
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
    if(m_noteCount >= m_voiceCount) {      
      // all voices were in use.. we might be able to 
      // reactivate a note that was previously overridden
      trig(
        m_notes[m_noteCount-m_voiceCount].note, 
        m_notes[m_noteCount-m_voiceCount].velocity
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
  // check if the note is already assigned to a channel...
  // if it is, then retrigger it
  for(int i=0; i<m_voiceCount; ++i) {
    if(m_voices[i].m_note == note) {
      m_voices[i].trig(note,velocity,this);    
      return;
    }
  }
  // Otherwise we should have a free channel
  for(int i=0; i<m_voiceCount; ++i) {
    if(!m_voices[i].m_note) {
      m_voices[i].trig(note,velocity,this);    
      return;
    }
  }
}  
///////////////////////////////////////////////////////////////////////
void CLogicalChannel::untrig(byte note) {
  for(int i=0; i<m_voiceCount; ++i) {
    if(m_voices[i].m_note == note) {
      m_voices[i].untrig(this);    
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
class CLogicalVoice 
{
//  CLogicalChannel *m_lch; //
  CPokeyChannel *m_pch;   // physical pokey channel
public:  
  byte m_note;  // MIDI note
  byte m_vel;  // MIDI velocity
  
  CLogicalVoice()
  {
    m_pch = NULL;
    m_note = 0;
    m_vel = 0;
  }

  void assign(CPokeyChannel *pch) 
  {
    m_pch = pch;
  } 
  void trig(byte note, byte vel)
  {
    m_note = note;
    m_vel = vel;
    update();
  }
  void untrig()
  {
    m_note = 0;
    m_vel = 0;
    update();
  }
  void update() 
  {
    if(!m_pch)
      return;
    if(m_note) {
      float freq = 440.0 * pow(2.0,((m_note-57.0)/12.0));
      m_pch->pitch(freq);
    }
    m_pch->vol(m_vel);
    //m_pch->pitch(440);
//    m_pch->vol(16);
  }
  
//  void untrig();                  
//  void detune(float amt);         // set detune amount for a voice (in fractional MIDI notes)
  
//  void run(CLogicalChannel *lch, unsigned long ms);     // run time based processing
};


class CLogicalChannel 
{
protected:  
  typedef struct {
    byte note;
    byte velocity;
  } NOTE;
  enum {
    MAX_NOTES = 10
  };
  CLogicalVoice *m_voices;
  byte m_voiceCount;
  NOTE m_notes[MAX_NOTES];
  byte m_noteCount;
public:  
  enum {
    FLAG_FULLVELOCITY    = 0x01
  };
  //byte   m_bendRange;               // pitch bend range
  byte   m_midiChannel;             // the midi channel for this logical channel;
  byte   m_flags;
  //float  m_deltaVolume;             // volume delta (for tremelo effect etc)
  //float  m_deltaPitch;              // pitch delta
  CLogicalChannel() 
  {
    m_flags = 0;//FLAG_FULLVELOCITY;
    m_voices = NULL;
    m_midiChannel = OMNI;
    m_noteCount = 0;
    m_voiceCount = 0;
  }
  ///////////////////////////////////////////////////////////////////////  
  void init(int voices)
  {
    if(m_voices) {
      delete[] m_voices;      
    }
    m_voiceCount = voices;
    m_voices = new CLogicalVoice[voices];
  }
  void assign(int voice, CPokeyChannel *pch)
  {
    if(!m_voices || voice < 0 || voice >= m_voiceCount)
      return;
    m_voices[voice].assign(pch);
  }  
  ///////////////////////////////////////////////////////////////////////
  byte deleteNote(byte note) {
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
  void handle(byte status, byte *params)
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
  }
  ///////////////////////////////////////////////////////////////////////
  void handleNoteOn(byte note, byte velocity)
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
  void handleNoteOff(byte note)
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
  void trig(byte note, byte velocity) {    
    // check if the note is already assigned to a channel...
    // if it is, then retrigger it
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_note == note) {
        m_voices[i].trig(note,velocity);    
        return;
      }
    }
    // Otherwise we should have a free channel
    for(int i=0; i<m_voiceCount; ++i) {
      if(!m_voices[i].m_note) {
        m_voices[i].trig(note,velocity);    
        return;
      }
    }
  }  
  ///////////////////////////////////////////////////////////////////////
  void untrig(byte note) {
    for(int i=0; i<m_voiceCount; ++i) {
      if(m_voices[i].m_note == note) {
        m_voices[i].untrig();    
        return;
      }
    }
  }
  
  //void run(unsigned long ms);               
  //void reset();                             
  
};




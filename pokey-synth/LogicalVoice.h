///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
class CLogicalChannel;

///////////////////////////////////////////////////////////////////////////////////////////
class CLogicalVoice 
{  
  // The logical channel number that this voice belongs to. This is an 
  // index into the global Channel[] array
  byte m_channel; 
  
  // The physical POKEY voice that this logical voice object is managing.
  // The voices on POKEY chip 1 are 0x00, 0x01, 0x02, 0x03
  // The voices on POKEY chip 2 are 0x80, 0x81, 0x82, 0x83
  byte m_voice;  

public:  
  enum {
    VOICE_POKEY2 = 0x80  // OR'd with the voice to indicate POKEY2 
  };

  // The MIDI note that this voice is playing (if any)
  char m_midi_note;             
  
  // The initial MIDI velocity of the note this voice is playing
  char m_midi_vel;

  // The state of the amplitude and modulation envelopes for this voice. 
  // Each voice has its own envelope states, but the envelope configurations 
  // are defined at the channel level and common to all voices on the channel
  ENVELOPE_STATE m_amp;
  ENVELOPE_STATE m_mod;
  
  // The detune factor for this voice. Detune "spreads out" voices which play
  // in unison by a certain amount. This factor multiplies the detune applied
  // to this specific voice (so each voice is detuned differently to the others)
  char  m_detuneFactor;

public:
  CLogicalVoice();
  void assign(byte channel, byte voice);
  void div8_high(byte v);
  void dist_poly9(byte v);
  void update();
  void reset();
  void quiet();
};

extern CLogicalVoice Voice[MAX_VOICE];


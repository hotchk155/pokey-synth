///////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL
//
// POKEYPIG
// hotchk155/2015
//
///////////////////////////////////////////////////////////


// Logical Channel object manages a set of "voices" that 
// play together, either as a polyphonic set of notes or
// a unison set of notes. All the voices play the same 
// POKEY sound and are triggered from the same MIDI channel

class CLogicalChannel 
{
  // The channel maintains a "note stack" of all the MIDI notes
  // that are currently "held" on the channel. This means that
  // when we run out of polyphony we can "steal" a voice from an
  // older note, but give it back whenif the newer note is released
  // first
  typedef struct {
    byte note;      // MIDI note (0-127)
    byte velocity;  // MIDI velocity (0-127)
  } NOTE;
  
  // This is the maximum number of MIDI notes that we can hold in 
  // the note stack at a time
  enum {
    MAX_NOTES = 10
  };  

  // The note stack
  NOTE m_notes[MAX_NOTES]; 
  
  // The number of notes in the stack
  byte m_noteCount; 

  // This channel manages a contiguous set of voices in the Voice[] array.
  // These members store the index of the first voice and 1 + the index of 
  // the last voice managed by this channel
  char m_voiceBegin; 
  char m_voiceEnd;   

  // Flags bits
  enum {
    SF_LFOSIGN      = 0x01,
    SF_LFOCOMPLETE  = 0x02,
//    SF_RECONFIG     = 0x04
  };
  
  // flags register
  byte m_flags;

  // Private methods
  void handleNoteOn(byte note, byte velocity);
  void handleNoteOff(byte note);
  void handleCC(char cc, char value);
  void handlePitchBend(byte lo, byte hi);
  void ccFlag(byte *flags, int flag, char value);
  void ccFlag(byte *flags, byte *negFlags, int flag, char value);  
  byte ccMapValue(char value, int maxValue);
  byte deleteNote(byte note);
  void trig(byte note, byte velocity, byte trigEnv);  
  void untrig(byte note);
  void recalc_detune();
  void runEnvelopes();
  void runLFO(); 
  void runPortamento(); 
  void runDetune(); 
  void runArpeggiator(); 

public:  
  
  // MIDI channel 0..15
  byte m_midiChannel;
  
  // Active note range in the channel
  char m_fromNote;
  char m_toNote;
  
  // Pointer to "patch" info
  TONE_CONFIG *m_conf;

  // State info
  float           m_fPitchBend;       // pich bend offset (semitones)
  float           m_fModWheel;        // mod wheel displacement (0.0 - 1.0)
  
  // Information calculated at the level of logical channel 
  float           m_fLFO;               // LFO in -1.0 to +1.0 range
  float           m_fLFOPositive;       // LFO in 0.0 to +1.0 range
  float           m_fPortamentoNote;    // current note played by portamento engine  
  float           m_fDetuneStep;        // detune step in semitones
  
  // Internal counters
  int             m_arpCounter;      // countdown until next arp step (8ms increments)
  byte            m_arpIndex;        // arp index in chord
  float           m_fLFOPhase;       // actual LFO count 0.0-1.0 value before wave applied
  char            m_portaTargetNote; // the target note of portamento engine 
  float           m_fPortaStep;      // portamento pitch change per 8ms
  
  CLogicalChannel();  
  void assign(byte voiceBegin, byte voiceEnd, TONE_CONFIG *conf);
  void handle(byte status, byte *params);
  void div8_high(byte v);
  void dist_poly9(byte v);
  void update(byte ticks);
  void start();  
  void quiet();
};

// Declare the global channel instances
extern CLogicalChannel Channel[MAX_CHANNEL];





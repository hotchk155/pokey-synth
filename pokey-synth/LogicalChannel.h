///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

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

  NOTE m_notes[MAX_NOTES];  // notes that are currently held
  byte m_noteCount;         // numer of notes that are held

  CLogicalVoice *m_voices; // pointer to the first logical voice assigned to this channel
  byte m_voiceCount;       // number of logical voices assigned to the channel

  
  
  void runEnvelopes();
  void runLFO(); 
  void runPortamento(); 
  void runDetune(); 
  void runArpeggiator(); 
    
public:  
  enum {
    SF_LFOSIGN      = 0x01,
    SF_LFOCOMPLETE  = 0x02,
    SF_RECONFIG     = 0x04
  };
  byte m_midiChannel;
  byte m_flags;
  TONE_CONFIG *m_conf;

  // State info
  float           m_fPitchBend;       // pich bend offset (semitones)
  float           m_fModWheel;        // mod wheel displacement (0.0 - 1.0)
  
  // Information calculated at the level of logical channel 
  float           m_fLFO;               // LFO in 0.0 to +1.0 range
  float           m_fLFOBipolar;        // LFO in -1.0 to +1.0 range
  float           m_fPortamentoNote;    // current note played by portamento engine  
  float           m_fDetuneStep;        // detune step in semitones
  
  // Internal counters
  int             m_arpCounter;      // countdown until next arp step (8ms increments)
  byte            m_arpIndex;        // arp index in chord
  float           m_fLFOCount;       // actual LFO count 0.0-1.0 value before wave applied
  char            m_portaTargetNote; // the target note of portamento engine 
  float           m_fPortaStep;      // portamento pitch change per 8ms
  
  CLogicalChannel();  
  void test();
  void assign(CLogicalVoice *voices, int voiceCount, TONE_CONFIG *conf);
  byte deleteNote(byte note);
  void handle(byte status, byte *params);
  void handleNoteOn(byte note, byte velocity);
  void handleNoteOff(byte note);
  void handleCC(char cc, char value);
  void handlePitchBend(byte lo, byte hi);
  void trig(byte note, byte velocity, byte trigEnv);  
  void untrig(byte note);
  void tick(byte counter) ;
  void ccFlag(byte *flags, int flag, char value);
  void ccFlag(byte *flags, byte *negFlags, int flag, char value);  
  byte ccMapValue(char value, int maxValue);
  void recalc_detune();
  void reset();
  void quiet();
  void div8_high(byte v);
  void dist_poly9(byte v);
  void update(byte ticks);
  void start();
  
};






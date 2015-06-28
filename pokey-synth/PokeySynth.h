///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
class CPokeySynth
{
  void defaultChannelConfig(char ch, CHANNEL_CONFIG *lc);
  void defaultGlobalConfig(GLOBAL_CONFIG *cfg);
  void loadGlobalConfig();
public:  

  // MIDI input manager
  CMidiInput m_midiInput;
  
  // Control panel (buttons, LEDs) manager
  CControlPanel m_controlPanel;    

  // The following objects manage the POKEY devices
  CPokey m_pokey1;
  CPokey m_pokey2;

  // This is the list of logical voices. Each one is a 
  // single "voice" that can be played by the synth, which
  // in turn controls one or two physical POKEY channels, 
  // depending on the mode the POKEY is configured in
  CLogicalVoice  m_logicalVoices[8];
  
  // List of the 4 logical channels. Each logical channel
  // controls a group of one or more logical voices as
  // assigned to it when the global config is loaded
  CLogicalChannel m_logicalChannels[NUM_LOGICAL_CHANNELS];

  // This structure contains the entire, currently active
  // "patch" for the synth and is the data that is stored
  // to EEPROM when patches are saved.
  GLOBAL_CONFIG m_globalConfig;
  
  CPokeySynth();
  void init();
  void run();
};




///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// This class represents a physical POKEY chip which
// has four physical voices (0..3), which can be paired up 
// for 16bit and HPF modes. 
class CPokey { 
  byte m_flags;
  byte m_data[8];    
  void write_reg(byte addr, byte data, byte force = 0);
  void reset();
public:    
  enum {
    MODE_8BIT = 0,
    MODE_8BITHPF,
    MODE_16BIT
  };
  CPokey(byte which);
  int configure(int mode, byte *voices);
  void pitch(byte voice, float hz);
  void vol(byte voice, float v);
  void dist(byte voice, float v);
  void hpf(byte voice, float v);
  void div8_high(byte voice, byte v);
  void dist_poly9(byte voice, byte v);
};


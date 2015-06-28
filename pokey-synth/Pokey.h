///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

class CPokey;
///////////////////////////////////////////////////////////
//
// POKEY PHYSICAL CHANNEL
//
/////////////////////////////////////////////////////////
class CPokeyChannel {

  byte m_mode;
  byte m_div_reg;    // address for freq divider register
  byte m_ctrl_reg;   // address for control register
  byte m_div2_reg;   // address for high freq divider register / hpf
  byte m_ctrl;   // data value for control register
  byte m_flags;
  CPokey *m_pokey;
  
  
public:    
  ///////////////////////////////////////////////////////////  
  // Channel modes
  enum {
    CHAN_NONE, // not configured
    CHAN_8,    // 8 bit
    CHAN_16,   // 16 bit
    CHAN_HPF   // 8 bit + hpf
  };
  enum {
    DIST_NONE  = 0b11100000,
    DIST_4     = 0b11000000,
    DIST_5     = 0b01100000,
    DIST_5_4   = 0b01000000,
    DIST_5_17  = 0b00000000,
    DIST_17    = 0b10000000,
    DIST_MASK  = 0b11100000,
  };
  enum {
    FLAG_NODIV = 0b00000001  // No divider set, so channel must be muted
  };
  
  ///////////////////////////////////////////////////////////  
  CPokeyChannel();
  void configure(CPokey *pokey, byte div, byte ctrl, byte div2);
  void mode(byte mode);
  void reset();
  void dist(int mode);
  void pitch(float hz);
  void hpf_lev(byte lev);  
  void dist_lev(byte lev);  
  void vol(byte level);
  void hpf(int hz);
  byte hz_to_div8(float hz);
  unsigned int hz_to_div16(float hz);
  void test();
  void range(byte v);
  
};

///////////////////////////////////////////////////////////
//
// POKEY PHYSICAL DEVICE
//
/////////////////////////////////////////////////////////
class CPokey {
  public:
    
    // POKEY write address
    enum {
      AUDF1   = 0x00, // Chan 1 frequency divider
      AUDC1   = 0x01, // Chanl 1 control register
      AUDF2   = 0x02, // Chan 2 frequency divider
      AUDC2   = 0x03, // Chanl 2 control register
      AUDF3   = 0x04, // Chan 3 frequency divider
      AUDC3   = 0x05, // Chanl 3 control register
      AUDF4   = 0x06, // Chan 4 frequency divider
      AUDC4   = 0x07, // Chanl 4 control register
      AUDCTL  = 0x08, // Master audio channel control
      STIMER  = 0x09, // Enable dividers
      SKREST  = 0x0a, // Reset serial status
      POTGO   = 0x0b, // Enable pot scan
      SEROUT  = 0x0d, // Serial tx reg
      IRQEN   = 0x0e, // Interrupt enable
      SKCTL   = 0x0f, // Serial port control  
      NO_REG  = 0xff  // not applicable place holder 
    };
    enum {
      AUDCTL_17BITPOLY         = 0x80,
      AUDCTL_CHAN1NODIV        = 0x40,
      AUDCTL_CHAN3NODIV        = 0x20,
      AUDCTL_CHAN1DIVSCASCADE  = 0x10,
      AUDCTL_CHAN3DIVSCASCADE  = 0x08,
      AUDCTL_CHAN1HPF          = 0x04,
      AUDCTL_CHAN3HPF          = 0x02,
      AUDCTL_DIVLOW            = 0x01
    };
    enum {
      PCFG_NONE      = 0,
      PCFG_8         = 1,   // supporting 4 channels with 8 bit divider
      PCFG_16        = 2,   // supporting two 16-bit channels
      PCFG_8_8_16    = 3,   // supporting two 8-bit and one 16 bit channel
      PCFG_8HPF      = 4,   // supporting 2 channels with 8 bit divider and 8 bit HPF
      PCFG_8_8HPF    = 5,   // supporting two 8-bit channels and one 8-bit channel with HPF
      PCFG_16_8HPF   = 6,   // supporting one 8-bit channel with HPF and one 16 bit channel            
      PCFG_8L        = 7,   // supporting 4 channels with 8 bit divider (low range)
      PCFG_8L_8L_16  = 8,   // supporting two 8-bit and one 16 bit channel  (low range)
      PCFG_8L_8LHPF  = 9,   // supporting two 8-bit channels and one 8-bit channel with HPF  (low range)
      PCFG_8LHPF     = 10,  // supporting 2 channels with 8 bit divider and 8 bit HPF  (low range)
      PCFG_16_8LHPF  = 11   // supporting one 8-bit channel with HPF and one 16 bit channel (low range)
    };
    enum {
      FLAG_LOWHZ             = 0x01  // Set dividers for 8 bit modes for low freq range
    };

    ///////////////////////////////////////////////////////////
    byte m_which;   // which pokey?
    byte m_mode;
    byte m_audctl;

    CPokeyChannel m_chan[4];
    
    ///////////////////////////////////////////////////////////
    CPokey(byte which);
    int configure(int mode, CPokeyChannel **channels);
    void write(byte addr, byte data);
    void audctl(byte v);
    void reset();
    void range(byte v);    
};


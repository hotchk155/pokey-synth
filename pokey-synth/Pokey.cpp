
///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
#include "Arduino.h"
#include "Defs.h"
#include "PortIO.h"
#include "Pokey.h"

// POKEY write address
enum {
  REG_AUDF1   = 0x00, // Chan 1 frequency divider
  REG_AUDC1   = 0x01, // Chanl 1 control register
  REG_AUDF2   = 0x02, // Chan 2 frequency divider
  REG_AUDC2   = 0x03, // Chanl 2 control register
  REG_AUDF3   = 0x04, // Chan 3 frequency divider
  REG_AUDC3   = 0x05, // Chanl 3 control register
  REG_AUDF4   = 0x06, // Chan 4 frequency divider
  REG_AUDC4   = 0x07, // Chanl 4 control register
  REG_AUDCTL  = 0x08, // Master audio channel control
  REG_STIMER  = 0x09, // Enable dividers
  REG_SKREST  = 0x0a, // Reset serial status
  REG_POTGO   = 0x0b, // Enable pot scan
  REG_SEROUT  = 0x0d, // Serial tx reg
  REG_IRQEN   = 0x0e, // Interrupt enable
  REG_SKCTL   = 0x0f, // Serial port control  
  REG_NONE  = 0xff  // not applicable place holder 
};
enum {
  AUDCTL_9BITPOLY          = 0x80,
  AUDCTL_CHAN1NODIV        = 0x40,
  AUDCTL_CHAN3NODIV        = 0x20,
  AUDCTL_CHAN1DIVSCASCADE  = 0x10,
  AUDCTL_CHAN3DIVSCASCADE  = 0x08,
  AUDCTL_CHAN1HPF          = 0x04,
  AUDCTL_CHAN2HPF          = 0x02,
  AUDCTL_DIVLOW            = 0x01
};
enum {
  DIST_NONE  = 0b11100000,
  DIST_4     = 0b11000000,
  DIST_5     = 0b01100000,
  DIST_5_4   = 0b01000000,
  DIST_5_17  = 0b00000000,
  DIST_17    = 0b10000000
};
enum {
  FLAG_DIV16    = 0x01,
  FLAG_HPF      = 0x02,
  FLAG_POKEY2   = 0x80,
};

const byte Control[4] =  {REG_AUDC1, REG_AUDC2, REG_AUDC3, REG_AUDC4};
const byte Divider[4] =  {REG_AUDF1, REG_AUDF2, REG_AUDF3, REG_AUDF4};
const byte Divider2[4] = {REG_NONE,  REG_AUDF1, REG_NONE,  REG_AUDF3};
const byte HighPass[4] = {REG_AUDF3, REG_AUDF4, REG_NONE,  REG_NONE};


///////////////////////////////////////////////////////////  
// FETCH 8 BIT DIVIDER VALUE FOR A SPECIFIC HZ FREQ IN LO RANGE
inline byte hz_to_div8_lo(float hz)
{
  if(hz<1) {
    return 0;
  }  
  int div =  (8769.0/hz) - 0.5;
  if(div < 0 || div > 255)
    return 0;
  return div;  
}

///////////////////////////////////////////////////////////  
// FETCH 8 BIT DIVIDER VALUE FOR A SPECIFIC HZ FREQ IN HI RANGE
inline byte hz_to_div8_hi(float hz)
{
  if(hz<1) {
    return 0;
  }  
  int div =  (35695.0/hz) - 0.5;
  if(div < 0 || div > 255)
    return 0;
  return div;  
}

///////////////////////////////////////////////////////////  
// FETCH 16 BIT DIVIDER VALUE FOR A SPECIFIC HZ FREQ 
inline unsigned int hz_to_div16(float hz)
{
  if(hz<1) {
    return 0;
  }  
  unsigned long div = 0.5 + 1000000.0/hz;
  if(div < 0 || div > 65535)
    return 0;
  return div;
}

///////////////////////////////////////////////////////////    
CPokey::CPokey(byte which) {
  m_flags = 0;
  if(which) {
    m_flags |= FLAG_POKEY2;
  }
  for(int i=0;i<16;++i) {
    m_data[i] = 0;    
  }
}

///////////////////////////////////////////////////////////
void CPokey::write_reg(byte addr, byte data, byte force) 
{ 
  // we only write values to addresses 0..REG_AUDCTL
  // if the value has changed from the previous write
  if(m_data[addr] <= REG_AUDCTL)
  {
    if(m_data[addr] == data && !force) {
      // same value
      return;
    }
    m_data[addr] = data;
  }
  
  // Perform the actual write of address/data pair
  _DIGITAL_WRITE(P_RW, LOW);  
  _DIGITAL_WRITE(P_AD0, !!(addr&0x01));
  _DIGITAL_WRITE(P_AD1, !!(addr&0x02));
  _DIGITAL_WRITE(P_AD2, !!(addr&0x04));
  _DIGITAL_WRITE(P_AD3, !!(addr&0x08));    
  _DIGITAL_WRITE(P_DB0, !!(data&0x01));
  _DIGITAL_WRITE(P_DB1, !!(data&0x02));
  _DIGITAL_WRITE(P_DB2, !!(data&0x04));
  _DIGITAL_WRITE(P_DB3, !!(data&0x08));
  _DIGITAL_WRITE(P_DB4, !!(data&0x10));
  _DIGITAL_WRITE(P_DB5, !!(data&0x20));
  _DIGITAL_WRITE(P_DB6, !!(data&0x40));
  _DIGITAL_WRITE(P_DB7, !!(data&0x80));      
  delayMicroseconds(10);
  if(m_flags & FLAG_POKEY2) {
    _DIGITAL_WRITE(P_CS1, LOW);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, LOW);
  }
  delayMicroseconds(10);
  if(m_flags & FLAG_POKEY2) {
    _DIGITAL_WRITE(P_CS1, HIGH);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, HIGH);
  }
}    

///////////////////////////////////////////////////////////    
void CPokey::reset() {
  for(int i=0;i<16;++i) {
    write_reg(i, 0, 1);
  }
  write_reg(REG_IRQEN,0);
}

///////////////////////////////////////////////////////////  
int CPokey::configure(int mode, byte *voices) {

  reset();  
  m_flags &= ~(FLAG_DIV16|FLAG_HPF);
  
  byte num_voices = 0;
  switch(mode) {
  case MODE_16BIT:
    m_flags |= FLAG_DIV16;
    write_reg(REG_AUDCTL, AUDCTL_CHAN1DIVSCASCADE | AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN1NODIV | AUDCTL_CHAN3NODIV);        
    voices[num_voices++] = 1;
    voices[num_voices++] = 3;
    break;
  case MODE_8BITHPF:
    m_flags |= FLAG_HPF;
    write_reg(REG_AUDCTL, AUDCTL_CHAN1HPF | AUDCTL_CHAN2HPF);        
    voices[num_voices++] = 0;
    voices[num_voices++] = 1;
    break;
  default:    
    voices[num_voices++] = 0;
    voices[num_voices++] = 1;
    voices[num_voices++] = 2;
    voices[num_voices++] = 3;
    break;
  }        
  write_reg(REG_STIMER, 1);        
  write_reg(REG_SKCTL, 3);
  return num_voices;
}

//////////////////////////////////////////////////
void CPokey::pitch(byte voice, float hz) {
    voice &= 3; // ensure voice is in 0..3 range!
    
    if(m_flags & FLAG_DIV16) {
      unsigned int v = hz_to_div16(hz);
      if(!v) {
        vol(voice,0);
      }
      else {
        write_reg(Divider[voice], v>>8);
        write_reg(Divider2[voice], (byte)v);
      }
    } 
    else {      
      byte v;
      if(m_data[REG_AUDCTL] & AUDCTL_DIVLOW) {
        v = hz_to_div8_lo(hz);
      }
      else {
        v = hz_to_div8_hi(hz);
      }
      if(!v) {
        vol(voice,0);
      }
      else {
        write_reg(Divider[voice], v);
      }
    }
}

//////////////////////////////////////////////////
void CPokey::vol(byte voice, float v) {
  voice &= 3;
  byte ctrl_reg = Control[voice];
  write_reg(ctrl_reg, (m_data[ctrl_reg] & 0xF0)|(byte)(0.5+15.0*v));
}

//////////////////////////////////////////////////
void CPokey::dist(byte voice, float v) {
  byte ctrl_reg = Control[voice&3];  
  switch((byte)(v*6.0)) {
    case 0:
      write_reg(ctrl_reg, (m_data[ctrl_reg] & 0x0F));
      break;
    case 1:
      write_reg(ctrl_reg, (m_data[ctrl_reg] & 0x0F)|DIST_4);
      break;
    case 2:
      write_reg(ctrl_reg, (m_data[ctrl_reg] & 0x0F)|DIST_5);
      break;
    case 3:
      write_reg(ctrl_reg, (m_data[ctrl_reg] & 0x0F)|DIST_5_4);
      break;
    case 4:
      write_reg(ctrl_reg, (m_data[ctrl_reg] & 0x0F)|DIST_5_17);
      break;
    case 5:
      write_reg(ctrl_reg, (m_data[ctrl_reg] & 0x0F)|DIST_17);
      break;
  }  
}

//////////////////////////////////////////////////
void CPokey::hpf(byte voice, float v) {
  if(m_flags & FLAG_HPF) {
    write_reg(HighPass[voice&3], v);
  }
}
  
//////////////////////////////////////////////////
void CPokey::div8_high(byte voice, byte v) {
    if(v) {
      write_reg(REG_AUDCTL, m_data[REG_AUDCTL] & ~AUDCTL_DIVLOW);
    }
    else {
      write_reg(REG_AUDCTL, m_data[REG_AUDCTL] | AUDCTL_DIVLOW);
    }
}

//////////////////////////////////////////////////
void CPokey::dist_poly9(byte voice, byte v) {
    if(v) {
      write_reg(REG_AUDCTL, m_data[REG_AUDCTL] | AUDCTL_9BITPOLY);
    }
    else {
      write_reg(REG_AUDCTL, m_data[REG_AUDCTL] & ~AUDCTL_9BITPOLY);
    }
}

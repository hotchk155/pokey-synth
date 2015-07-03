///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
#include "Arduino.h"
#include "Defs.h"
#include "PortIO.h"
#include "Pokey.h"

///////////////////////////////////////////////////////////  
//
// PHYSICAL POKEY CHANNEL
//
///////////////////////////////////////////////////////////  

///////////////////////////////////////////////////////////  
CPokeyChannel::CPokeyChannel() {
  m_mode = CHAN_NONE;
  m_div_reg = CPokey::NO_REG;
  m_ctrl_reg = CPokey::NO_REG;
  m_div2_reg = CPokey::NO_REG;
  m_hpf_reg = CPokey::NO_REG;
  m_ctrl = DIST_NONE;
  m_flags = FLAG_NODIV;
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::configure(CPokey *pokey, byte div_reg, byte ctrl_reg, byte div2_reg, byte hpf_reg) {
  m_div_reg = div_reg;
  m_ctrl_reg = ctrl_reg;
  m_div2_reg = div2_reg;
  m_hpf_reg = hpf_reg;
  m_flags = FLAG_NODIV;
  m_pokey = pokey;
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::test() {
  m_pokey->write(8, B01010000); // lsbit 0=higher 1=lower freq
  m_pokey->write(0, 2); //lsb
  m_pokey->write(1, 0);
  m_pokey->write(2, 10); //msb
  m_pokey->write(3, 0b10101111);
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::mode(byte mode) {
  m_mode = mode;
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::reset() {
  m_ctrl = DIST_NONE;  
  m_flags = FLAG_NODIV;
  quiet();
} 
///////////////////////////////////////////////////////////  
void CPokeyChannel::quiet() {
  m_pokey->write(m_div_reg, 0);
  m_pokey->write(m_ctrl_reg, 0);
  if(m_div2_reg != CPokey::NO_REG) {
    m_pokey->write(m_div2_reg, 0);
  }
}
///////////////////////////////////////////////////////////  
byte CPokeyChannel::hz_to_div8(float hz)
{
  if(hz<1) {
    return 0;
  }  
  unsigned long v;
  if(m_pokey->m_audctl & CPokey::AUDCTL_DIVLOW) {
    v = hz*452;
  }
  else {
    v = hz*113;
  }  
  int div = 4000000.0/v;
  if(div < 0 || div > 255)
    return 0;
  return div;
}

///////////////////////////////////////////////////////////  
unsigned int CPokeyChannel::hz_to_div16(float hz)
{
  if(hz<1) {
    return 0;
  }  
  unsigned long div = 1000000.0/hz;
  if(div < 0 || div > 65535)
    return 0;
  return div;
}

///////////////////////////////////////////////////////////  
void CPokeyChannel::dist(int mode) {
  byte v = m_ctrl & ~DIST_MASK;
  v|=(mode & DIST_MASK);
  if(v != m_ctrl) {
    m_ctrl = v;
    m_pokey->write(m_ctrl_reg, m_ctrl);
  }
}
///////////////////////////////////////////////////////////  
// Accept a level 0-127 and set appropriate dist level
void CPokeyChannel::dist_lev(byte level) {
    switch((int)(level/22.0))
    {
      case 1: dist(DIST_4); break;
      case 2: dist(DIST_5); break;
      case 3: dist(DIST_5_4); break;
      case 4: dist(DIST_5_17); break;
      case 5: dist(DIST_17); break;
      default: dist(DIST_NONE); break;
    }
}

///////////////////////////////////////////////////////////  
void CPokeyChannel::pitch(float hz) {    unsigned int v;
  switch(m_mode) {
    case CHAN_8:
    case CHAN_HPF:
      v = hz_to_div8(hz);
      if(v) {
        m_pokey->write(m_div_reg, v);
        m_flags &= ~FLAG_NODIV;
        return;
      }
      break;      
    case CHAN_16:    
      v = hz_to_div16(hz);
      if(v) {
        m_pokey->write(m_div_reg, (v>>8));
        m_pokey->write(m_div2_reg, (v & 0x00FF));
        m_flags &= ~FLAG_NODIV;
        return;
      }
      break;
  }
  // bad divider
  m_flags |= FLAG_NODIV;
  vol(0);
}

///////////////////////////////////////////////////////////  
// Volume is provided as 0-15
void CPokeyChannel::vol(byte level) {
  if(m_flags & FLAG_NODIV) {
    level = 0;
  }
  byte v = m_ctrl & 0xF0;
  v|=(level & 0x0F);
  if(v != m_ctrl) {
    m_ctrl = v;
    m_pokey->write(m_ctrl_reg, m_ctrl);
  }  
}

///////////////////////////////////////////////////////////  
void CPokeyChannel::hpf(int hz) {
  if(m_mode == CHAN_HPF) {
    byte v = hz_to_div8(hz);
    m_pokey->write(m_hpf_reg, v);
  }
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::hpf_lev(byte lev) {
  if(m_mode == CHAN_HPF) {
    m_pokey->write(m_hpf_reg, lev);
  }
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::range(byte v) {
  m_pokey->range(v);
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::poly9(byte v) {
  m_pokey->poly9(v);
}

///////////////////////////////////////////////////////////  
//
// PHYSICAL POKEY DEVICE
//
///////////////////////////////////////////////////////////  

///////////////////////////////////////////////////////////  
CPokey::CPokey(byte which) 
{
  m_which = which;
  m_mode = PCFG_NONE;
  m_audctl = 0;
  reset();
  
  m_chan[0].configure(this, AUDF1, AUDC1, NO_REG, AUDF3);
  m_chan[1].configure(this, AUDF2, AUDC2, AUDF1,  AUDF4);
  m_chan[2].configure(this, AUDF3, AUDC3, NO_REG, NO_REG);
  m_chan[3].configure(this, AUDF4, AUDC4, AUDF3,  NO_REG);
}
///////////////////////////////////////////////////////////  
int CPokey::configure(int mode, byte lowhz, CPokeyChannel **channels) {
  int i;
  for(i=0; i<4; ++i) {
    m_chan[i].reset();      
    channels[i] = NULL;
  }
  byte divlow = lowhz? AUDCTL_DIVLOW:0;
  switch(mode) {
  case PCFG_8:
    audctl(divlow);
    for(i=0; i<4; ++i) {
      m_chan[i].mode(CPokeyChannel::CHAN_8);
      channels[i] = &m_chan[i];
    }
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 4;
  case PCFG_8HPF:
    audctl(divlow | CPokey::AUDCTL_CHAN1HPF | CPokey::AUDCTL_CHAN2HPF);        
    m_chan[0].mode(CPokeyChannel::CHAN_HPF);
    m_chan[1].mode(CPokeyChannel::CHAN_HPF);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[1];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;
  case PCFG_8_8HPF:
    audctl(divlow | AUDCTL_CHAN2HPF);        
    m_chan[0].mode(CPokeyChannel::CHAN_HPF);
    m_chan[1].mode(CPokeyChannel::CHAN_8);
    m_chan[3].mode(CPokeyChannel::CHAN_8);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[1];
    channels[2] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 3;      
  case PCFG_8_8_16:  
    audctl(divlow | CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN3NODIV);        
    m_chan[0].mode(CPokeyChannel::CHAN_8);
    m_chan[1].mode(CPokeyChannel::CHAN_8);
    m_chan[3].mode(CPokeyChannel::CHAN_16);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[1];
    channels[2] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 3;      
/*  case PCFG_16_8HPF: 
    audctl(divlow | CPokey::AUDCTL_CHAN1HPF | CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN3NODIV);        
    m_chan[0].mode(CPokeyChannel::CHAN_HPF);
    m_chan[3].mode(CPokeyChannel::CHAN_16);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;      */
  case PCFG_16:
    audctl(divlow | CPokey::AUDCTL_CHAN1DIVSCASCADE | CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN1NODIV | AUDCTL_CHAN3NODIV);        
    m_chan[1].mode(CPokeyChannel::CHAN_16);
    m_chan[3].mode(CPokeyChannel::CHAN_16);
    channels[0] = &m_chan[1];
    channels[1] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;          
    
  }
  return 0;
}
///////////////////////////////////////////////////////////
void CPokey::write(byte addr, byte data) 
{      
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
  if(m_which) {
    _DIGITAL_WRITE(P_CS1, LOW);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, LOW);
  }
  delayMicroseconds(10);
  if(m_which) {
    _DIGITAL_WRITE(P_CS1, HIGH);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, HIGH);
  }
}    
///////////////////////////////////////////////////////////
void CPokey::audctl(byte v) {
  write(AUDCTL, v);
  m_audctl = v;
}
///////////////////////////////////////////////////////////
void CPokey::reset() {
  for(int i=0;i<16;++i) {
    write(i, 0);
  }
  write(IRQEN,0);
}
///////////////////////////////////////////////////////////
void CPokey::quiet() {
  for(int i=0;i<4;++i) {
    m_chan[i].quiet();
  }
}

///////////////////////////////////////////////////////////
void CPokey::range(byte v) {
  if(v) {
    if(m_audctl & AUDCTL_DIVLOW) {
      audctl(m_audctl & ~AUDCTL_DIVLOW);
    }
  } 
  else {    
    if(!(m_audctl & AUDCTL_DIVLOW)) {
      audctl(m_audctl | AUDCTL_DIVLOW);
    }
  }
}
    
void CPokey::poly9(byte v) {
  if(v) {
    if(!(m_audctl & AUDCTL_9BITPOLY)) {
      audctl(m_audctl | AUDCTL_9BITPOLY);
    }
  } 
  else {    
    if(m_audctl & AUDCTL_9BITPOLY) {
      audctl(m_audctl & ~AUDCTL_9BITPOLY);
    }
  }
}


///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
#define HEARTBEAT_PERIOD 20
class CControlPanel {
  byte m_heartbeat;
  unsigned long m_nextHeartbeat;
public:
  CControlPanel() {
    m_heartbeat = 0;
    m_nextHeartbeat = 0;
  }
  void pulse() 
  {
    digitalWrite(P_LED1, 1);
    m_nextHeartbeat = millis() + HEARTBEAT_PERIOD;
  }
  void run(unsigned long ms) {
    if(m_nextHeartbeat && ms > m_nextHeartbeat) {
      m_nextHeartbeat = 0;
      digitalWrite(P_LED1, 0);
    }
  }
};


///////////////////////////////////////////////////////////////////////////////////////////
//
// POKEY SYNTH
//
// hotchk155/2015
//
///////////////////////////////////////////////////////////////////////////////////////////

#include "Arduino.h"

#include "PortIO.h"
#include "PokeySynth.h"
#include "Pokey.h"
#include "MidiInput.h"
#include "ControlPanel.h"
#include "LogicalChannel.h"

CPokey Pokey1(0);
CPokey Pokey2(1);

CPokeyChannel *Pokey1Channels[4] = {
  0};
byte Pokey1NumChannels = 0;
CPokeyChannel *Pokey2Channels[4] = {
  0};
byte Pokey2NumChannels = 0;


CLogicalChannel LogicalChannel1;
//CLogicalVoice LogicalVoice1;

void setup()
{
  pinMode(P_AD0, OUTPUT);
  pinMode(P_AD1, OUTPUT);
  pinMode(P_AD2, OUTPUT);
  pinMode(P_AD3, OUTPUT);

  pinMode(P_DB0, OUTPUT);
  pinMode(P_DB1, OUTPUT);
  pinMode(P_DB2, OUTPUT);
  pinMode(P_DB3, OUTPUT);
  pinMode(P_DB4, OUTPUT);
  pinMode(P_DB5, OUTPUT);
  pinMode(P_DB6, OUTPUT);
  pinMode(P_DB7, OUTPUT);

  pinMode(P_CS0, OUTPUT);
  pinMode(P_CS1, OUTPUT);
  pinMode(P_RW, OUTPUT);

  pinMode(P_LED1, OUTPUT);
  pinMode(P_LED2, OUTPUT);

  digitalWrite(P_RW, LOW);
  digitalWrite(P_CS1, HIGH);
  digitalWrite(P_CS0, HIGH);


  Pokey1NumChannels = Pokey1.configure(CPokey::MODE_16, Pokey1Channels);
  //  Pokey1.range(false);
  //Pokey1Channels[0]->pitch(300);
  //Pokey1Channels[0]->vol(255);
  //  LogicalVoice1.assign(Pokey1Channels[0]);
  //  LogicalVoice1.trig(61,127);
  LogicalChannel1.init(1);
  LogicalChannel1.assign(0, Pokey1Channels[0]);
//  LogicalChannel1.assign(1, Pokey1Channels[1]);
//  LogicalChannel1.assign(2, Pokey1Channels[2]);
//  LogicalChannel1.assign(3, Pokey1Channels[3]);

  //PokeyChannel[0]->test();
  //Pokey1Channels[0]->pitch(300);
  //Pokey1Channels[0]->vol(255);

  MidiInput.init();
  //Serial.begin(31250);
  //Serial.flush();
}

unsigned long lastTick = 0;
void loop() 
{
  unsigned long ms = millis();
  ControlPanel.run(ms);

  byte midi = MidiInput.read();
  if(midi)
  {
    ControlPanel.pulse();
    LogicalChannel1.handle(midi, MidiInput.m_params);
  }
  if(lastTick != ms) {
    lastTick = ms;
    LogicalChannel1.tick();
  }
}




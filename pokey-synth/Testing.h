void failTest(int test) {
  pinMode(P_LED1, OUTPUT);
  for(;;) {
    for(int i=0; i<test; ++i) {
      digitalWrite(P_LED1, HIGH);
      delay(200);
      digitalWrite(P_LED1, LOW);
      delay(200);
    }
    delay(1000);
  }
}  


void pokeyTest1()
{
  byte voice[4];
  int v;
  
  v = Pokey1.configure(CPokey::MODE_8BIT, voice);   
  TEST_CONDITION(v==4, 1);
//  Pokey1.test();
  
  Pokey1.pitch(voice[0], 440);
  Pokey1.vol(voice[0], 1.0);
  Pokey1.pitch(voice[1], 550);
  Pokey1.vol(voice[1], 1.0);

  delay(1000);
  Pokey1.vol(voice[0],0);  
  Pokey1.vol(voice[1],0);  
}  

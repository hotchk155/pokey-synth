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

void indicateTest(int test) {
  pinMode(P_LED2, OUTPUT);
  digitalWrite(P_LED2, !!test);  
}


void pokeyTest1()
{
  byte voice[4];
  int v;
  float i;
  
  v = Pokey1.configure(CPokey::MODE_8BIT, voice);   
  TEST_CONDITION(v==4, 1);
//  Pokey1.test();
  
  Pokey1.pitch(voice[0], 440);
  for(i = 0.0; i<=1.0; i+=0.05) {
    Pokey1.vol(voice[0], i);
    delay(100);
  }
  
  delay(1000);
  Pokey1.pitch(voice[1], 550);
  for(i = 0.0; i<=1.0; i+=0.05) {
    Pokey1.vol(voice[1], i);
    delay(100);
  }

  delay(1000);
  Pokey1.pitch(voice[2], 660);
  for(i = 0.0; i<=1.0; i+=0.05) {
    Pokey1.vol(voice[2], i);
    delay(100);
  }

  delay(1000);
  Pokey1.pitch(voice[3], 700);
  for(i = 0.0; i<=1.0; i+=0.05) {
    Pokey1.vol(voice[3], i);
    delay(100);
  }

  delay(5000);
  Pokey1.vol(voice[0],0);  
  Pokey1.vol(voice[1],0);  
  Pokey1.vol(voice[2],0);  
  Pokey1.vol(voice[3],0);  
}  

void pokeyTest2()
{
  byte voice[4];
  int v;
  
  v = Pokey1.configure(CPokey::MODE_8BIT, voice);   
  TEST_CONDITION(v==4, 1);
//  Pokey1.test();

  for(int i=0; i<4; ++i)
  {  
    Pokey1.pitch(voice[i], 400 + 50 * i);
    Pokey1.vol(voice[i], 1.0);  
    delay(1000);
    for(float j=0.0; j<=1.0; j += 0.01) {  
      Pokey1.dist(voice[i], j);
      delay(100);
    }
    Pokey1.vol(voice[i],0);  
  }
}  

void pokeyTest3()
{
  byte voice[4];
  int v;
  
  v = Pokey1.configure(CPokey::MODE_8BITHPF, voice);   
  TEST_CONDITION(v==2, 1);
//  Pokey1.test();

  for(int i=0; i<2; ++i)
  {  
    Pokey1.pitch(voice[i], 400 + 100 * i);
    Pokey1.vol(voice[i], 1.0);  
    delay(1000);
    for(float j=0.0; j<=1.0; j += 0.01) {  
      Pokey1.hpf(voice[i], j);
      delay(100);
    }
    Pokey1.vol(voice[i],0);  
  }
}  

void pokeyTest4()
{
  byte voice[4];
  int v;
  
  v = Pokey1.configure(CPokey::MODE_16BIT, voice);   
  TEST_CONDITION(v==2, 1);
//  Pokey1.test();

  for(int j=100; j<500; ++j)
  {  
    Pokey1.pitch(voice[0], j);
    Pokey1.pitch(voice[1], j*3);
    Pokey1.vol(voice[0], 1.0);  
    Pokey1.vol(voice[1], 1.0);  
    delay(10);
  }
  Pokey1.vol(voice[0], 0);  
  Pokey1.vol(voice[1], 0);  
}  

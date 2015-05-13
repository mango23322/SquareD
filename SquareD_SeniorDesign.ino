const byte numPins = 8;
unsigned long time1;
unsigned long time2;
int LEDpin = 2;
byte startBit = (byte)'*';
byte stopBit =  (byte)'?';
int startstop = 0;
int outnum = 0;
int length = 0;
int count = 0;
int j = 0;
int circuitID = 0;
int starCount = 0;
// ascii to dec is number plus 48, only for 0-9
byte circuit[66] = {
                (byte)'*',49,(byte)'*',49,48,(byte)'*',55,(byte)'*',48,(byte)'*',48,
                (byte)'*',50,(byte)'*',48,(byte)'*',48,(byte)'*',48,(byte)'*',48,
                (byte)'*',51,(byte)'*',53,(byte)'*',51,(byte)'*',48,(byte)'*',48,
                (byte)'*',52,(byte)'*',49,48,(byte)'*',55,(byte)'*',48,(byte)'*',48,
                (byte)'*',53,(byte)'*',57,(byte)'*',54,(byte)'*',48,(byte)'*',48,
                (byte)'*',54,(byte)'*',49,48,(byte)'*',55,(byte)'*',55,(byte)'*',48,(byte)'*'};
int lightArray[50];

byte test[11] = {(byte)'T',(byte)'e',(byte)'s',(byte)'t',(byte)'e',32,(byte)'B',(byte)'y',(byte)'t',(byte)'e',(byte)'.'};

int lightStart = 0;
int lightEnd = 0;
int lightPrev = 0;
int light2Prev = 0;
int lightCurrent = 0;

void setup() 
{
  pinMode(LEDpin,OUTPUT);
  pinMode(A0,INPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  Serial.begin(9600);
}

void loop() 
{
  circuitID = lightInput(); // check for incoming data from phone
  
  if(circuitID != 0) // if a code is recieved, send that breaker's info
  {
    circuitID--;
    starCount = 0;
    int counter = circuitID*5;
    counter++;
    
    if(circuitID >= 0 && circuitID <= 5) // check if the circuit ID is valid
    {
      for(j = 0; starCount < counter; j++)
      {
        byte currentVal = circuit[j];
        if(currentVal == (byte)'*')
        {
          starCount++;     
        }  
      }   
      j=j-1; //returns to the last star that was seen because it is the start bit.
      
      Serial.print(j);
      Serial.print("  ");
      if(startstop == 0) // prints out start bit.
      {
        startstop = 1; 
        sendOne();
        //Serial.print("Start bit:");
        Serial.print(" ");
        printByte(startBit);
        Serial.print(" ");
      }
   
      for(starCount = 0; starCount< 6; j++)
      {
        byte currentVal = circuit[j];
        if(currentVal == (byte)'*') // check if the current byte is a star
        {
          starCount++;
        }
        if(starCount < 6) //ensures that the star bit that acts as the start bit for the next circuit breaker is not used.
        {
          printByte(circuit[j]);
          Serial.print(" ");
        }
      }
      if(startstop == 1) // send stop bit
      {
        startstop = 0;
        printByte(stopBit);
        Serial.print(" ");
        sendOne();
        Serial.println("");
      }
    }
    else 
      Serial.println("Invalid Circuit ID. Try again?");
      
    circuitID = 0;
  }
  
  
}  

void printByte(byte sendByte)
{  
  for (byte i=0; i<numPins; i++)
  {
    byte state = bitRead(sendByte, 7-i); // read the current bit
    
    if (state == 1) // if the bit is high, send a 1
    {
      sendOne();
    }
    else // if the bit is low, send a zero
    {
      sendZero();
    }
  }
}

void sendOne()
{
    digitalWrite(LEDpin,HIGH); // turon on for 15 ms, then off for the rest of the 33.33 ms
    delay(15);
    digitalWrite(LEDpin,LOW);
    delay(18);
    delayMicroseconds(333);
    Serial.print("1");
}

void sendZero()
{
    digitalWrite(LEDpin,LOW);
    delay(33);
    delayMicroseconds(333);
    Serial.print("0");
}


int getValue()
{
  int value = analogRead(A0);
  //Serial.println(value);
  if(value >= 500) // threshold of the on/off signal
  {
    return 1;
  }
  else
    return 0;
}

int lightInput()
{
  int timeDiff;
  if(getValue() == 0)
    return 0;
  
  time1 = millis();
  
  while(getValue() == 1)
  {
    //wait for light to turn off, then measure the delay
    time2 = millis();
    timeDiff = time2 - time1;
    if(timeDiff > 4000)
    {
      flashLEDS();
      return 0;
    }
  }
  time2 = millis();
  
  timeDiff = time2 - time1;
  Serial.print("start diff:");
  Serial.print(timeDiff);
  if(timeDiff > 20 && timeDiff < 350) //(was 130 indicates that this was the start bit //avg is 115ms
  {
    time1 = millis();
    while(getValue() == 0)
    {
      //wait for light to turn back on, then measure
      time2 = millis();
      timeDiff = time2 - time1;
      if(timeDiff > 4000)
      {
        flashLEDS();
        return 0;
      }
    }
    time1 = millis();
   // Serial.print(",Time1:");
   // Serial.print(time1);
    while(getValue() == 1)
    {
      //wait or light to turn off then measure time difference
      time2 = millis();
      timeDiff = time2 - time1;
      if(timeDiff > 4000)
      {
        flashLEDS();
        return 0;
      }
    }
    time2 = millis();
    timeDiff = time2 - time1;
    Serial.println(", time diff 2:");
    Serial.println(timeDiff);
    for(int p = 1; p < 8;p++)
    {
      int qp = 500*p;
      if(timeDiff > (qp-150) && timeDiff < (qp+150))
      {
       Serial.print("Code recieved");
       digitalWrite(5,HIGH);
       digitalWrite(6,HIGH);
       digitalWrite(7,HIGH);
       digitalWrite(8,HIGH);
       digitalWrite(9,HIGH);
       delay(1000);
       digitalWrite(9,LOW);
       delay(1000);
       digitalWrite(8,LOW);
       delay(1000);
       digitalWrite(7,LOW);
       delay(1000);
       digitalWrite(6,LOW);
       delay(1000);
       digitalWrite(5,LOW);
       delay(1000);
       return (p);
      }
    }
      flashLEDS();
  }
  
  flashLEDS();
  return 0;
}

void flashLEDS()
{
  int dtime = 250;
  for(int inti = 0; inti < 5; inti++)
  {
    digitalWrite(9,HIGH);
    digitalWrite(8,HIGH);
    digitalWrite(7,HIGH);
    digitalWrite(6,HIGH);
    digitalWrite(5,HIGH);
    delay(dtime);
    digitalWrite(9,LOW);
    digitalWrite(8,LOW);
    digitalWrite(7,LOW);
    digitalWrite(6,LOW);
    digitalWrite(5,LOW);
    delay(dtime); 
  }
}
    


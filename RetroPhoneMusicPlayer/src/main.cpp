#include <Arduino.h>
#include <DFMiniMp3.h>

#include <SoftwareSerial.h>

class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char *action)
  {
    if (source & DfMp3_PlaySources_Sd)
    {
      Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb)
    {
      Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash)
    {
      Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);
  }
  static void OnPlaySourceOnline(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};

//DFMiniMp3<HardwareSerial, Mp3Notify> mp3(Serial1);

SoftwareSerial secondarySerial(10, 11); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

//Input declarations

int rotaryDialInput = 2;
int handsetRest = 3;
int testProbe = 4;
volatile unsigned long currTime;
volatile unsigned long prevTime;
int pulseCounter;
boolean toggle = false;
volatile boolean rotaryDialTrigger = false;
int handsetRestStatus;
volatile int handsetTrigger;

void selectSong()
{
  Serial.print("Num dialled:");
  Serial.println(pulseCounter);
  Serial.print("track:");
  Serial.println(pulseCounter);
  if (handsetRestStatus == false)
  {
    mp3.playMp3FolderTrack(pulseCounter); // sd:/mp3/0001.mp3
    pulseCounter = 0;
  }
}

void rotaryDialISR()
{
  rotaryDialTrigger = true;
}

void handsetResetISR()
{

  handsetTrigger = true;
}

void setup()
{
  prevTime = millis();
  Serial.begin(115200);

  Serial.println("initializing...");
  Serial.println("Input defines");
  pinMode(rotaryDialInput, INPUT); //Maybe we can use external pullups later.
  pinMode(handsetRest, INPUT_PULLUP);
  //pinMode(testProbe, OUTPUT);
  //Consider external pullups for use with battery.
  //Remember to check rise times with higher pullup resistor values.
  attachInterrupt(digitalPinToInterrupt(rotaryDialInput), rotaryDialISR, RISING);
  attachInterrupt(digitalPinToInterrupt(handsetRest), handsetResetISR, CHANGE);

  //mp3
  mp3.begin();
  mp3.setVolume(17);
  mp3.setEq(DfMp3_Eq_Normal);
  uint16_t volume = mp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);

  uint16_t count = mp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);

  Serial.println("starting...");
}

unsigned long currTriggerTime = 0;
unsigned long prevTriggerTime = 0;

void loop()
{

 if (handsetTrigger == true)
  {
    Serial.println("Handset Reset");
    delay(2);
    handsetRestStatus = digitalRead(handsetRest);
    mp3.playFolderTrack(01,001);
    if (handsetRestStatus == true)
    {
      mp3.stop();
      digitalWrite(testProbe, true);
      _delay_ms(10);
      digitalWrite(testProbe, false);
      handsetRestStatus = false;
    }
    handsetTrigger = false;
    
  }

  if (rotaryDialTrigger == true)
  {
    prevTriggerTime = currTriggerTime;
    currTriggerTime = millis();
    rotaryDialTrigger = false;

    if (currTriggerTime - prevTriggerTime > 70)
    {
      if (currTriggerTime - prevTriggerTime < 100)
      {
        //Last pulse, you can count number of pulses here.
        selectSong();
      }
      else
      {
        pulseCounter++;
      }
    }
  }
  
 //mp3.playMp3FolderTrack(6);
 //delay(10000);
 //mp3.playFolderTrack(001,001);
 //delay(10000);
}
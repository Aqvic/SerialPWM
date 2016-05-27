/*
*   Simple 4-bytes serial protocol: 
*   RX: [COMMAND] [PARAM] [VALUE] [XOR]   TX: [OK|VALUE|ERROR]
*   
*   COMMAND             PARAM                 VALUE       RETURNS
*   ------------------  -------------------   ---------   ---------
*   0x00 (setPWMValue)  0x00-0x03 (pwmPin)    0x00-0xFF   OK|ERROR
*   0x01 (setPWMFreq)   0xOO-0x04 (pwmFreq)   ---         OK|ERROR
*   0x02 (getEEPROM)    0x00-0xFF (address)   ---         VALUE
*   0x03 (setEEPROM)    0x00-0xFF (address)   0x00-0xFF   VALUE
*
*   Status and errors codes see below in constants section
*/

#include <Arduino.h>
#include <EEPROM.h>

// Use static const instead DEFINE. Because it respects scope and is type-safe!
// Use static const instead MAGIC NUMBERS. Always!

static const byte COMMAND = 0;
static const byte PARAM   = 1;
static const byte VALUE   = 2;
static const byte XOR     = 3;

static const byte PROTO_MAX_COMMANDS = 4;
static const byte PROTO_RX_BUFFER_SIZE = 4;

static const byte PROTO_OK = 0x00;

static const byte PROTO_ERROR_BAD_COMMAND = 0xE0;
static const byte PROTO_ERROR_WRONG_PARAM = 0xE1;
static const byte PROTO_ERROR_WRONG_VALUE = 0xE3;
static const byte PROTO_ERROR_XOR         = 0xE4;
static const byte PROTO_ERROR_RX_FAULT    = 0xE5;

static const byte pwmPinsNumber = 4;
static const byte pwmPins[pwmPinsNumber] = {9, 10 ,11, 5}; // manual initialization

byte setPWMValue(byte data[]){
  if(data[PARAM]<pwmPinsNumber){
    analogWrite(pwmPins[data[PARAM]], data[VALUE]);
    return PROTO_OK;
  }
  return PROTO_ERROR_WRONG_PARAM;
}


void set31KHzPWM(){
    TCCR2B = TCCR2B & B11111000 | B00000001; //Timer2 D3, D11
    TCCR1B = TCCR1B & B11111000 | B00000001; //Timer1 D9, D10
}

void set4KHzPWM(){
    TCCR2B = TCCR2B & B11111000 | B00000010;
    TCCR1B = TCCR1B & B11111000 | B00000010;
}

void set500HzPWM(){
    TCCR2B = TCCR2B & B11111000 | B00000100;
    TCCR1B = TCCR1B & B11111000 | B00000011;
}

void set120HzPWM(){
    TCCR2B = TCCR2B & B11111000 | B00000110;
    TCCR1B = TCCR1B & B11111000 | B00000100;
}

void set30HzPWM(){
    TCCR2B = TCCR2B & B11111000 | B00000111;
    TCCR1B = TCCR1B & B11111000 | B00000101;
}

void (*pwmFrequency[5])() = {set30HzPWM, set120HzPWM, set500HzPWM, set4KHzPWM, set31KHzPWM};


byte setPWMFrequency(byte data[]){
  if(data[PARAM]<5){
    (*pwmFrequency[data[PARAM]])();
    return PROTO_OK;
  }
  return PROTO_ERROR_WRONG_PARAM;
}


byte getEEPROM(byte data[]){
  return EEPROM.read(data[PARAM]);
}

byte setEEPROM(byte data[]){
  EEPROM.update(data[PARAM], data[VALUE]);
  return EEPROM.read(data[PARAM]);
}

byte (*proto[PROTO_MAX_COMMANDS])(byte data[]) = {setPWMValue, setPWMFrequency, getEEPROM, setEEPROM};


void setup() {

  pinMode(13, OUTPUT); // board Led

  for(int i=0; i<pwmPinsNumber; i++){
    pinMode(pwmPins[i], OUTPUT);
    analogWrite(pwmPins[i], 0);
  }
    
  Serial.begin(57600);
  Serial.setTimeout(100);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}


void serialEvent() {

  byte buf[PROTO_RX_BUFFER_SIZE] = {0xff, 0xff ,0xff, 0xff}; // manual initialization
        
  if(Serial.readBytes(buf, PROTO_RX_BUFFER_SIZE) == PROTO_RX_BUFFER_SIZE){
    if(checkXOR(buf)){
      if(buf[COMMAND] < PROTO_MAX_COMMANDS){
        Serial.write((*proto[buf[COMMAND]])(buf));
      }else{
        Serial.write(PROTO_ERROR_BAD_COMMAND);
      }
    }else{
      Serial.write(PROTO_ERROR_XOR);
    }
  }else{
    Serial.write(PROTO_ERROR_RX_FAULT);
  }

  while (Serial.available() > 0){Serial.read();} // clear serial buffer
}

bool checkXOR(byte data[]){

  byte checksum = data[0];
  for(int i=1; i<PROTO_RX_BUFFER_SIZE-1; i++){
    checksum ^= data[i];
  }
  
  return (data[XOR] == checksum);  
}





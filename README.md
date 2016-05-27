# Serial PWM
## Simple serial protocol for testing Arduino PWM

```
Simple 4-bytes serial protocol: 
RX: [COMMAND] [PARAM] [VALUE] [XOR]   TX: [OK|VALUE|ERROR]
   
COMMAND             PARAM                 VALUE       RETURNS
------------------  -------------------   ---------   ---------
0x00 (setPWMValue)  0x00-0x03 (pwmPin)    0x00-0xFF   OK|ERROR
0x01 (setPWMFreq)   0xOO-0x04 (pwmFreq)   ---         OK|ERROR
0x02 (getEEPROM)    0x00-0xFF (address)   ---         VALUE
0x03 (setEEPROM)    0x00-0xFF (address)   0x00-0xFF   VALUE

Status and Errors codes:
---------------------------------------------------------------
PROTO_OK                = 0x00;
PROTO_ERROR_BAD_COMMAND = 0xE0;
PROTO_ERROR_WRONG_PARAM = 0xE1;
PROTO_ERROR_WRONG_VALUE = 0xE3;
PROTO_ERROR_XOR         = 0xE4;
PROTO_ERROR_RX_FAULT    = 0xE5;

Example
---------------------------------------------------------------
Set duty cycle to 50% (0x80) on first  defined pwm pin:
Send to Arduino (hex): 00 00 80 80
Answer from Arduino (hex): 00
```

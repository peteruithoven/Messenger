#ifndef Messenger_h
#define Messenger_h

#include "Arduino.h"
#include <SoftwareSerial.h>

typedef void (*MessageHandler)(int type, int value);

#define MAX_TRANSMIT_BUFFER 64
#define MESSAGE_LENGTH 2

class Messenger
{
  public:
    Messenger(SoftwareSerial* serial, int txPin, MessageHandler messageHandler, int ledPin);
    void sendMessage(char type, int value);
    void update();
	
  private:
	
	SoftwareSerial* _serial;
    MessageHandler _messageHandler;
	
    int _txPin;
	int _ledPin;
	int _recMessageType;
	int _recMessageValue;
	int _recMessageChecksum;
	int _lightCounter;
	boolean _waiting;
	byte _transmit_buffer[MAX_TRANSMIT_BUFFER][MESSAGE_LENGTH]; 
	byte _transmit_buffer_tail;
	byte _transmit_buffer_head;
	long resendTime;
	void readMessage(int message);
};

#endif

#ifndef Messenger_h
#define Messenger_h

#include "Arduino.h"
#include <SoftwareSerial.h>

typedef void (*MessageHandler)(int type, int value);

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
	int _messageType;
	int _messageValue;
	int _recMessageType;
	int _recMessageValue;
	
	int _lightCounter;
	
	void readMessage(int message);
};

#endif

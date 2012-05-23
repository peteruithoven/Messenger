#ifndef Messenger_h
#define Messenger_h

#include "Arduino.h"
#include <SoftwareSerial.h>

typedef void (*MessageHandler)(int type, int value,int i);

#define MAX_TRANSMIT_BUFFER 64
#define MESSAGE_LENGTH 2
#define MESSENGER_DEBUG false

class Messenger
{
  public:
    Messenger(int i,SoftwareSerial* serial, int txPin, MessageHandler messageHandler, int ledPin);
    void sendMessage(char type, int value, bool overrideSameType);
    void update();
	void listen();
	
  private:
	
	SoftwareSerial* _serial;
    MessageHandler _messageHandler;
	
	int _id;
    int _txPin;
	int _ledPin;
	int _recMessageType;
	int _recMessageValue;
	int _recMessageChecksum;
	int _lightCounter;
	boolean _waiting;
	byte _transmit_buffer[MAX_TRANSMIT_BUFFER][MESSAGE_LENGTH]; 
	byte _transmit_buffer_tail;
	byte _transmit_buffer_prev_tail;
	byte _transmit_buffer_head;
	long resendTime;
	long sendMessageTime;
	
	void readMessage(int message);
};

#endif

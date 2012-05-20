#include "Arduino.h"
#include "Messenger.h"
#include <SoftwareSerial.h>

Messenger::Messenger(SoftwareSerial* serial, int txPin, MessageHandler messageHandler, int ledPin)
{
	//_serial;
	_serial = serial;
	_messageHandler = messageHandler;
	_txPin = txPin;
	_ledPin = ledPin;
	
	_serial->begin(4800);
	
	_messageType = -1;
	_messageValue = -1;
	_recMessageType = -1;
	_recMessageValue = -1;
	_recMessageChecksum = -1;
	
	_lightCounter = 0;
}
void Messenger::sendMessage(char type, int value)
{
	_messageType = type;
	_messageValue = value;
}
void Messenger::update()
{
	if(_messageType != -1) // resend time, (recieved message is complete?)
	{
		pinMode(_txPin,OUTPUT); // enable TX / output
		_serial->write(_messageType);
		_serial->write(_messageValue);
		_serial->write('\n');
		pinMode(_txPin,INPUT); // disable TX / output
		
		_messageType = -1;
		_messageValue = -1;
		
		digitalWrite(_ledPin, LOW);  
	}
	else
	{
		if(_serial->available() > 0)
		{
			_lightCounter = 50;
			while(_serial->available() > 0)
				readMessage(_serial->read());
		}
		digitalWrite(_ledPin, HIGH);  
	}
	
	//_lightCounter--;
	//if(_lightCounter < 0) _lightCounter = 0; 
	//digitalWrite(_ledPin, ((_lightCounter > 0)? LOW : HIGH));  
}
void Messenger::readMessage(int message)
{
	if(message == '\n')
	{
		_recMessageType = -1;
		_recMessageValue= -1;
		_recMessageChecksum = -1;
	}
	else if(_recMessageType == -1)
	{
		_recMessageType = message;
	}
	else if(_recMessageValue == -1)
	{
		_recMessageValue = message;
	}
	else if(_recMessageChecksum == -1)
	{
		_recMessageChecksum = message;
		(*_messageHandler)(_recMessageType, _recMessageValue);
	}
}
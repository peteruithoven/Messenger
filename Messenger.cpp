#include "Arduino.h"
#include "Messenger.h"
#include <SoftwareSerial.h>

Messenger::Messenger(int i,SoftwareSerial* serial, int txPin, MessageHandler messageHandler, int ledPin)
{
	//_serial;
	_id = i;
	_serial = serial;
	_messageHandler = messageHandler;
	_txPin = txPin;
	_ledPin = ledPin;
	
	pinMode(txPin,INPUT); // Setting TX to input so that it doesn't block RX
	pinMode(ledPin,OUTPUT);
	
	_serial->begin(9600);
	_recMessageType = -1;
	_recMessageValue = -1;
	_recMessageChecksum = -1;
	_lightCounter = 0;
	_waiting = false;
	
	_transmit_buffer_head = _transmit_buffer_tail = _transmit_buffer_prev_head = 0;
}
void Messenger::sendMessage(char type, int value, bool overrideSameType) 
{	
	if(value == 10) return; //weird stuf...
	
	if(MESSENGER_DEBUG) 
	{
		Serial.print("S");
		Serial.print(" ");
		Serial.print(type);
		Serial.print(value);
		
		int numMessages = 0;
		if(_transmit_buffer_tail <_transmit_buffer_head)
			numMessages = _transmit_buffer_head-_transmit_buffer_tail;
		else if(_transmit_buffer_tail > _transmit_buffer_head)
			numMessages = _transmit_buffer_head + (MAX_TRANSMIT_BUFFER-_transmit_buffer_tail);
		Serial.print(" ");
		Serial.print(_transmit_buffer_tail);
		Serial.print(" ");
		Serial.print(_transmit_buffer_head);
		Serial.print(" ");
		Serial.println(numMessages);
	}
	if(type=='@')
	{
		int temp_transmit_buffer_tail = _transmit_buffer_tail-1;
		if(temp_transmit_buffer_tail < 0) temp_transmit_buffer_tail = MAX_TRANSMIT_BUFFER-1;
		
		if (temp_transmit_buffer_tail == _transmit_buffer_head)
		{
			// when the buffer is full I'm overriding messages because 
			// the sooner the message from the other party are confirmed the sooner he is done sending.
			temp_transmit_buffer_tail = _transmit_buffer_tail;
			Serial.print("o");
			Serial.println("t");
		}
		_transmit_buffer[temp_transmit_buffer_tail][0] = type; 
		_transmit_buffer[temp_transmit_buffer_tail][1] = value;
		_transmit_buffer_tail = temp_transmit_buffer_tail;
	}
	else
	{
		if(overrideSameType && _transmit_buffer[_transmit_buffer_prev_head][0] == type)
		{
			_transmit_buffer[_transmit_buffer_prev_head][1] = value;
		}
		else
		{
			if ((_transmit_buffer_head + 1) % MAX_TRANSMIT_BUFFER != _transmit_buffer_tail) 
			{
				// save new message in buffer: tail points to where byte goes
				_transmit_buffer[_transmit_buffer_head][0] = type; 
				_transmit_buffer[_transmit_buffer_head][1] = value;
				_transmit_buffer_prev_head = _transmit_buffer_head;
				_transmit_buffer_head = (_transmit_buffer_head + 1) % MAX_TRANSMIT_BUFFER;
			} 
			else 
			{
				// overflow
				Serial.println("o");
			}
		}
	}
}
void Messenger::update()
{
	//Serial.println("  u");
	//if(_messageType != -1) // resend time, (recieved message is complete?)
	
	if (_transmit_buffer_tail != _transmit_buffer_head && (!_waiting || (_waiting && millis()>resendTime))) // buffer not empty?
	{
		if(MESSENGER_DEBUG && _waiting && millis()>resendTime) Serial.println("r");
		
		byte* data = _transmit_buffer[_transmit_buffer_tail]; // grab next byte
		
		//int sumData = (data[0]+data[1])/2;
		//byte checksum = (sumData/256) ^ (sumData&0xFF);
		byte checksum = (data[0]+data[1])/2;
		//Serial.println(checksum);
		
		if(MESSENGER_DEBUG) 
		{
			Serial.print("T");
			Serial.print(" ");
			//Serial.print(data[0]);
			Serial.write(data[0]);
			Serial.print(data[1]);
			Serial.print(" ");
			Serial.println(checksum);
		}
		pinMode(_txPin,OUTPUT); // enable TX / output
		_serial->write('\n');
		_serial->write(data[0]);
		//int message = 'E';
		//_serial->write(message);
		_serial->write(data[1]);
		_serial->write(checksum);
		pinMode(_txPin,INPUT); // disable TX / output
		
		//if(!_waiting || (_waiting && millis()>resendTime))
		//	sendMessageTime = millis();
		
		if(data[0] == '@') // if confirmation remove
		{
			_waiting = false;
			_transmit_buffer_tail = (_transmit_buffer_tail + 1) % MAX_TRANSMIT_BUFFER;
		}
		else // else wait for confirmation 		
		{
			_waiting = true;
			// when the other atmega isn't listening we have to send it again
			// with a direct connection it should take about 20 / 21 milliseconds
			resendTime = millis()+random(40,80); //TODO try more, might fix similtanious transmit issue
			//40,200: 10-62
			//40,80: 10-51 (gem: 15)
		}
		digitalWrite(_ledPin, LOW);  
		//_lightCounter = 200;
	}
	else
	{
		if(_serial->available() > 0)
		{
			//Serial.println(_serial->available());
			//_lightCounter = 50;
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
	//Serial.print(" ");
	//Serial.print("R");
	//Serial.print(" ");
	//Serial.println(message);
	//Serial.print(" ");
	if(message == '\n')
	{
		_recMessageType		= -1;
		_recMessageValue	= -1;
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
		
		if ((_recMessageType+_recMessageValue)/2 == _recMessageChecksum)
		{
			if(_recMessageType == '@')
			{
				// value == checksum is head message?
				
				(*_messageHandler)(_recMessageType, _recMessageValue, _id);
				
				byte* data = _transmit_buffer[_transmit_buffer_tail];
				byte checksum = (data[0]+data[1])/2;
				
				//Serial.print(checksum);
				//Serial.print(" ");
				//Serial.println(_recMessageValue);
				
				if(_recMessageValue == checksum)
				{
					if(MESSENGER_DEBUG) Serial.println("c");
					_transmit_buffer[_transmit_buffer_tail][0] = 0;
					_transmit_buffer_tail = (_transmit_buffer_tail + 1) % MAX_TRANSMIT_BUFFER;
					_waiting = false;
					
					//Serial.print('c');
					//Serial.print(' ');
					//Serial.println(millis()-sendMessageTime);
				}
			}
			else
			{
				(*_messageHandler)(_recMessageType, _recMessageValue, _id);
				sendMessage('@',_recMessageChecksum,false);
			}
		}
		else 
		{
			Serial.println("e");
		}
	}
}
void Messenger::listen()
{
	_serial->listen();
}
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
	
	_serial->begin(9600);
	_recMessageType = -1;
	_recMessageValue = -1;
	_recMessageChecksum = -1;
	_lightCounter = 0;
	_waiting = false;
	
	_transmit_buffer_tail = _transmit_buffer_head = 0;
}
void Messenger::sendMessage(char type, int value)
{
	//_serial->write(type);
	
	Serial.print("S");
	Serial.print(" ");
	if(value == '@') Serial.write(value);
	else Serial.print(value);
	//Serial.println(type);
	Serial.println(" ");
	
	
	if ((_transmit_buffer_tail + 1) % MAX_TRANSMIT_BUFFER != _transmit_buffer_head) 
    {
		// save new message in buffer: tail points to where byte goes
		_transmit_buffer[_transmit_buffer_tail][0] = type; 
		_transmit_buffer[_transmit_buffer_tail][1] = value;
		_transmit_buffer_tail = (_transmit_buffer_tail + 1) % MAX_TRANSMIT_BUFFER;
    } 
    else 
    {
		// overflow
		Serial.println("o");
    }
}
void Messenger::update()
{
	//if(_messageType != -1) // resend time, (recieved message is complete?)
	
	if (_transmit_buffer_head != _transmit_buffer_tail && (!_waiting || (_waiting && millis()>resendTime))) // buffer not empty?
	{
		if(_waiting && millis()>resendTime) Serial.println("r");
		
		byte* data = _transmit_buffer[_transmit_buffer_head]; // grab next byte
		
		//int sumData = (data[0]+data[1])/2;
		//byte checksum = (sumData/256) ^ (sumData&0xFF);
		byte checksum = (data[0]+data[1])/2;
		//Serial.println(checksum);

		Serial.print("T");
		Serial.print(" ");
		//Serial.write(data[0]);
		Serial.print(data[1]);
		//Serial.print(" ");
		//Serial.print(checksum);
		
		pinMode(_txPin,OUTPUT); // enable TX / output
		_serial->write(data[1]);
		//int message = 'E';
		//_serial->write(message);
		//_serial->write(data[1]);
		//_serial->write(checksum);
		//_serial->write('\n');
		pinMode(_txPin,INPUT); // disable TX / output
		
		if(data[0] == '@') // if confirmation remove
		{
			_transmit_buffer_head = (_transmit_buffer_head + 1) % MAX_TRANSMIT_BUFFER;
		}
		else // else wait for confirmation
		{
			_waiting = true;
			resendTime = millis()+random(1000,2000);//500,1000);
		}
		
		//Serial.print(" ");
		//Serial.println(_waiting);
		Serial.println(" ");
		digitalWrite(_ledPin, LOW);  
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
	//Serial.print("R");
	//Serial.print(" ");
	//Serial.println(message);
	//Serial.print(" ");
	/*if(message == '\n')
	{
		//Serial.println("a");
		_recMessageType		= -1;
		_recMessageValue	= -1;
		_recMessageChecksum = -1;
	}
	else if(_recMessageType == -1)
	{
		//Serial.println("b");
		_recMessageType = message;
	}
	else if(_recMessageValue == -1)
	{
		//Serial.println("c");
		_recMessageValue = message;
	}
	else if(_recMessageChecksum == -1)
	{
		//Serial.println("d");
		_recMessageChecksum = message;
		
		if ((_recMessageType+_recMessageValue)/2 == _recMessageChecksum)
		{
			//Serial.print("R");
			//Serial.print(" ");*/
	
			_recMessageValue = message;
	
			//if(_recMessageType == '@')
			if(_recMessageValue == '@')
			{
				// value == checksum is head message?
				
				//(*_messageHandler)(_recMessageType, _recMessageValue);
				
				/*byte* data = _transmit_buffer[_transmit_buffer_head];
				byte checksum = (data[0]+data[1])/2;
				//Serial.print(checksum);
				//Serial.print(" ");
				//Serial.println(_recMessageValue);
				if(_recMessageValue == checksum)
				{
					//Serial.println("c");*/
					_transmit_buffer_head = (_transmit_buffer_head + 1) % MAX_TRANSMIT_BUFFER;
				//	_waiting = false;
				//}
			}
			else
			{
				(*_messageHandler)(_recMessageType, _recMessageValue, _id);
				//sendMessage('@',_recMessageChecksum);
				sendMessage('@','@');
			}
		/*}
		else 
		{
			//Serial.println("E");
		}
	}*/
}
void Messenger::listen()
{
	_serial->listen();
}
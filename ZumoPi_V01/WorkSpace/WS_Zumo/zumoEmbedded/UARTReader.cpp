#include "UARTReader.h"

//#define DEBUGPRINT

UARTReader::UARTReader(Stream& serialPort, char *startStr, char stopChar, int maxBufSize) : serialPort(serialPort) {
	this->UARTMode  = UART_SYNCING;
  this->bufIndex = 0; // static ptr for when data comes in separate chunks
  this->syncSteps = 0;
  this->startStr = new char[1 + strlen(startStr)];
  strcpy(this->startStr, startStr);
	this->syncDone = (int)strlen(startStr);
  this->stopChar = stopChar;
  this->inputBuffer = new char[maxBufSize];
}

UARTReader::~UARTReader() {
  delete this->startStr;
  delete this->inputBuffer;
}

int UARTReader::getMessage() {

  int nBytes, bytesRead;
  char c;
  
  if(!(nBytes = serialPort.available()))
    return UARTMode;

  bytesRead = 0;
  while( bytesRead < nBytes ) {
    switch(UARTMode) {
      case UART_SYNCING:
        while(bytesRead < nBytes && UARTMode == UART_SYNCING) {
          c = (char)serialPort.read();
#ifdef DEBUGPRINT
            Serial.write(c);
#endif 					
					bytesRead++;
          syncSteps = (c == startStr[syncSteps] ? syncSteps + 1 : 0);
          if(syncSteps == syncDone) {  // sync'ed //
            bufIndex = syncSteps = 0;
            UARTMode = UART_RECEIVING;
          }
        }
        break;
      case UART_RECEIVING:
        while(bytesRead < nBytes && UARTMode == UART_RECEIVING) {
          c = (char)serialPort.read();
#ifdef DEBUGPRINT
            Serial.write(c);
#endif
          inputBuffer[bufIndex] = c;
					bytesRead++;
					bufIndex++;
          if(c == stopChar) {  // we got a packet!! //
#ifdef DEBUGPRINT
            Serial.write("Package");
#endif
            inputBuffer[bufIndex] = NULL; 
            bufIndex = 0;
            UARTMode = UART_SYNCING;
            return UART_GOT_MESSAGE;
          }
          if(bufIndex == MAX_INPUT_BUFFER-2) { // overflow
            bufIndex = 0;
            UARTMode = UART_SYNCING;
            return UART_BUFF_OVERFLOW;
          } 
        }
      break;
    } // switch //
  } // while //
  return UARTMode; // UART_SYNCING or UART_RECEIVING
}

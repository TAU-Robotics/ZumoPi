/*
  UARTReadBytes.h - Library for non-blocking read from UART comm
*/

#ifndef UARTReader_h
#define UARTReader_h
#include <Arduino.h>

// Structure of a single frame:
// delimiter + json message + '\n'

// Buffer size including delimiters: 512 bytes 
#define MAX_INPUT_BUFFER 512

class UARTReader
{
  public:
    UARTReader(Stream& serialPort, char *startDelim, char stopChar, int maxBufSize=MAX_INPUT_BUFFER);
   ~UARTReader();
    int getMessage();
    char* buffer() { return inputBuffer; }

    static int const UART_GOT_MESSAGE = 0;
    static int const UART_SYNCING = 1;
    static int const UART_RECEIVING = 2;
    static int const UART_BUFF_OVERFLOW = 3;
    
  private:
    char* inputBuffer;
    char* startStr; // = "json:";
    char stopChar; 
    
    int UARTMode;
    int bufIndex; // static ptr for when data comes in separate chunks
    int syncSteps;
		int syncDone;
    
    Stream& serialPort;

};

#endif

#include <string.h>
#include <stdlib.h>

//this following macro is good for debugging, e.g.  print2("myVar= ", myVar);
#define print2(x,y) (Serial.print(x), Serial.println(y))


#define CR '\r'
#define LF '\n'
#define BS '\b'
#define NULLCHAR '\0'
#define SPACE ' '

#define COMMAND_BUFFER_LENGTH        511        // length of serial buffer for incoming commands
char   commandLine[COMMAND_BUFFER_LENGTH + 1];  // Read commands into this buffer from Serial.  +1 in length for a termination char

const char *delimiters            = " \t\0";

bool getCommandLineFromSerialPort() {
  static uint16_t charsRead = 0;

  while ( Serial.available() > 0 ) {
    char c = Serial.read();
    switch (c) {
      case CR: case LF:
        commandLine[charsRead] = NULLCHAR;
        charsRead = 0;
        return true;
        break;
      case BS: case 127:
        if (charsRead > 0) {
          commandLine[--charsRead] = NULLCHAR;
          Serial.print(BS);
          Serial.print(' ');
          Serial.print(BS);
        }
        break;
      default:
        // c = tolower(c);
        if (charsRead < COMMAND_BUFFER_LENGTH) {
          commandLine[charsRead++] = c;
          Serial.print(c);
        }
        commandLine[charsRead] = NULLCHAR;     //just in case
        break;
    }
  }
  return false;
}


String getFirstWord() {
  char * word = strtok(commandLine, delimiters);
  return ( word == 0 ) ? "" : String(word);
}

String getNextWord() {
  char * word = strtok(NULL, delimiters);
  return ( word == 0 ) ? "" : String(word);
}

String getRemainder() {
  char * word = strtok(NULL, "");
  String ret;
  if ( word == 0 ) {
    ret = "";
  } else {
    ret = String(word);
    ret.trim();
  }
  return ret;
}

uint8_t asciiHexToUint8(String asciiHex) {
  uint8_t result = 0;
  int value;
  sscanf(asciiHex.c_str(), "%x", &value);
  result = value;
  return result;
}
// String uint8ToAsciiHex(uint8_t value) {
//   String asciiHex = new String(sprintf("%02x", value));
//   return asciiHex;
// }


// /* ****************************
//    readNumber: return a 16bit (for Arduino Uno) signed integer from the command line
//    readWord: get a text word from the command line
// */
// int readNumber () {
//     char * numTextPtr = getWordChar();
//     return atoi(numTextPtr);
// }


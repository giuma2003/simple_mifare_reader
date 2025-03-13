#ifndef _funcs_hpp_
#define _funcs_hpp_

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>


byte getNdefPrefix(const String& url);
void createNdefMessage(const String& url, byte* buffer, int& length);

bool writeBlock(MFRC522& mfrc522, byte block, byte *data);
bool readBlock(MFRC522 mfrc522, byte block, byte *buffer);

void printCardBlocks(MFRC522 mfrc522, MFRC522::MIFARE_Key key);


#endif // _funcs_hpp_
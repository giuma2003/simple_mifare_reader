#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include "funcs.hpp"

#define SS_PIN 15   	// D8 (GPIO2) - SDA
#define RST_PIN 2 		// D4 (GPIO15) - Reset

MFRC522 mfrc522(SS_PIN, RST_PIN); // Creazione dell'oggetto MFRC522

String url = "https://www.google.com";
byte ndefBuffer[64];
int ndefLength = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    SPI.begin();        // Inizializza SPI
    mfrc522.PCD_Init(); // Inizializza il modulo RC522

	createNdefMessage(url, ndefBuffer, ndefLength);

	Serial.println("NDEF generato:");
    for (int i = 0; i < ndefLength; i++) {
        Serial.print("0x");
        Serial.print(ndefBuffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void loop() {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) 
        return;

    Serial.println("Carta rilevata!");

    byte block = 4;  // Scriviamo nel primo blocco disponibile

	if (writeBlock(mfrc522, block, ndefBuffer)) 
		Serial.println("NFC scritto con successo! Ora prova a leggere con un telefono.");
	else 
		Serial.println("Errore nella scrittura NFC.");
	
	byte readBuffer[64];

	if (readBlock(mfrc522, block, readBuffer)) {
		Serial.println("NFC letto con successo!");
		Serial.print("Dati letti: ");
		for (int i = 0; i < ndefLength; i++) {
			Serial.print(readBuffer[i], HEX);
			Serial.print(" ");
		}
		Serial.println();
	} 
	else 
		Serial.println("Errore nella lettura NFC.");
	

    mfrc522.PICC_HaltA(); 
    mfrc522.PCD_StopCrypto1();
}
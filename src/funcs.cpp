
#include "funcs.hpp"


void printCardBlocks(MFRC522 mfrc522, MFRC522::MIFARE_Key key) {
    Serial.println("\n=== Lettura della carta ===");

    for (byte sector = 0; sector < 16; sector++) { // 16 settori totali su MIFARE Classic 1K
        Serial.print("\n[ Settore ");
        Serial.print(sector);
        Serial.println(" ]");

        for (byte block = 0; block < 4; block++) { // Ogni settore ha 4 blocchi
            byte blockNumber = sector * 4 + block;
            byte buffer[18];
            byte bufferSize = sizeof(buffer);

            // Autenticazione necessaria prima di leggere
            MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
                MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid)
            );
            if (status != MFRC522::STATUS_OK) {
                Serial.print("Autenticazione fallita sul blocco ");
                Serial.print(blockNumber);
                Serial.print(": ");
                Serial.println(mfrc522.GetStatusCodeName(status));
                continue;
            }

            // Lettura del blocco
            status = mfrc522.MIFARE_Read(blockNumber, buffer, &bufferSize);
            if (status != MFRC522::STATUS_OK) {
                Serial.print("Errore di lettura sul blocco ");
                Serial.print(blockNumber);
                Serial.print(": ");
                Serial.println(mfrc522.GetStatusCodeName(status));
                continue;
            }

            // Stampa il blocco
            Serial.print("Blocco ");
            Serial.print(blockNumber);
            Serial.print(": ");
            for (byte i = 0; i < 16; i++) {
                Serial.print(buffer[i] < 0x10 ? " 0" : " ");
                Serial.print(buffer[i], HEX);
            }
            Serial.println();
        }
    }

    // Ferma la comunicazione con la carta
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

// Mappa dei prefissi NDEF per URL
byte getNdefPrefix(const String& url) {
    if (url.startsWith("http://www.")) return 0x01;
    if (url.startsWith("https://www.")) return 0x02;
    if (url.startsWith("http://")) return 0x03;
    if (url.startsWith("https://")) return 0x04;
    return 0x00;  // Nessun prefisso, URL assoluto
}

// Funzione per generare dinamicamente il vettore di byte NDEF
void createNdefMessage(const String& url, byte* buffer, int& length) {
    byte prefix = getNdefPrefix(url);
    String cleanUrl = url;

    // Rimuoviamo il prefisso dall'URL per risparmiare spazio
    if (prefix == 0x01) cleanUrl.remove(0, 11);  // "http://www."
    else if (prefix == 0x02) cleanUrl.remove(0, 12);  // "https://www."
    else if (prefix == 0x03) cleanUrl.remove(0, 7);  // "http://"
    else if (prefix == 0x04) cleanUrl.remove(0, 8);  // "https://"

    int urlLen = cleanUrl.length();
    length = urlLen + 7;  // Dimensione totale del messaggio

    buffer[0] = 0x03;           // Inizio messaggio
    buffer[1] = length - 2;     // Lunghezza totale meno i due byte iniziali
    buffer[2] = 0xD1;           // Header
    buffer[3] = 0x01;           // Tipo lunghezza (1 byte)
    buffer[4] = urlLen + 1;     // Lunghezza del payload (URL + 1 byte per il prefisso)
    buffer[5] = 0x55;           // Tipo (55 = URL)
    buffer[6] = prefix;         // Prefisso URL

    // Copia dell'URL nel buffer
    for (int i = 0; i < urlLen; i++) {
        buffer[7 + i] = cleanUrl[i];
    }

    buffer[7 + urlLen] = 0xFE;  // Fine del messaggio
}

bool writeBlock(MFRC522& mfrc522, byte block, byte *data) {
    MFRC522::StatusCode status;
    
    // Autenticazione del blocco con chiave di default (0xFFFFFFFFFFFF)
    byte key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    MFRC522::MIFARE_Key mifareKey;
    memcpy(mifareKey.keyByte, key, 6);

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &mifareKey, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Errore di autenticazione: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Scrittura del blocco
    status = mfrc522.MIFARE_Write(block, data, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Errore di scrittura: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    return true;
}

bool readBlock(MFRC522 mfrc522, byte block, byte *buffer) {
    MFRC522::StatusCode status;
    byte bufferSize = 18;

    // Autenticazione del blocco
    byte key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    MFRC522::MIFARE_Key mifareKey;
    memcpy(mifareKey.keyByte, key, 6);

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &mifareKey, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Errore di autenticazione: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Lettura del blocco
    status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Errore di lettura: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    return true;
}


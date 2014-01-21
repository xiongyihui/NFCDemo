// NFC Printer
//
// Use NFC peer to peer communication
// The sketch depends on PN532 library(https://github.com/Seeed-Studio/PN532)
// and NDEF library(https://github.com/don/NDEF)


#include "SPI.h"
#include "PN532_SPI.h"
#include "snep.h"
#include "NdefMessage.h"
#include "SoftwareSerial.h"

PN532_SPI pn532spi(SPI, 10);
SNEP nfc(pn532spi);
uint8_t ndefBuf[128];
uint8_t recordBuf[128];
SoftwareSerial printer(3, 4);

void setup()
{
    Serial.begin(115200);
    Serial.println("-------NFC Printer--------");
    printer.begin(9600);
}

void loop()
{

    Serial.println("Get a message from Android");
    int msgSize = nfc.read(ndefBuf, sizeof(ndefBuf));
    if (msgSize > 0) {
        NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
        msg.print();
        Serial.println("\nSuccess");

        NdefRecord record = msg.getRecord(0);
        record.print();
        int recordLength = record.getPayloadLength();
        if (recordLength <= sizeof(recordBuf)) {
            record.getPayload(recordBuf);
            printer.write(recordBuf, recordLength);
            
            for (int i = 0; i < 5; i++) {
                printer.write('\n');
            }
            delay(2000);
        }
    } else {
        Serial.println("failed");
    }
    delay(1000);
}  

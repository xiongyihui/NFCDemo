#include <PN532.h>
#include <NFCLinkLayer.h>
#include <SNEP.h>

#include <NdefMessage.h>

#include "SoftwareSerial.h"

SoftwareSerial printer(3, 4);

#define SCK 13
#define MOSI 11
#define SS 10
#define MISO 12

PN532 nfc(SCK, MISO, MOSI, SS);
NFCLinkLayer linkLayer(&nfc);
SNEP snep(&linkLayer);

// This message shall be used to rx or tx 
// NDEF messages it shall never be released
#define MAX_PKT_HEADER_SIZE  50
#define MAX_PKT_PAYLOAD_SIZE 100
uint8_t rxNDEFMessage[MAX_PKT_HEADER_SIZE + MAX_PKT_PAYLOAD_SIZE];
uint8_t *rxNDEFMessagePtr; 

void setup(void) {
    
    Serial.begin(115200);
    Serial.println("-------- NFC Printer ---------");
    printer.begin(9600);
    
    nfc.initializeReader();

    // configure board to read RFID tags and cards
    nfc.SAMConfig();
}

void loop(void) 
{
    uint32_t rxResult = GEN_ERROR; 
    rxNDEFMessagePtr = &rxNDEFMessage[0];
    
     Serial.println("Peer to peer: as a target");
     if (IS_ERROR(nfc.configurePeerAsTarget(SNEP_SERVER))) {
       extern uint8_t pn532_packetbuffer[];
       
       while (!nfc.isReady()) {
       }
       
       nfc.readspicommand(PN532_TGINITASTARGET, (PN532_CMD_RESPONSE *)pn532_packetbuffer);
     }
    
    do {
        rxResult = snep.rxNDEFPayload(rxNDEFMessagePtr);
        
        if (rxResult == SEND_COMMAND_RX_TIMEOUT_ERROR)
        {
           break;
        } else if (IS_ERROR(rxResult)) {
          break;
        }
   
       
    
        if (RESULT_OK(rxResult))
        {
           NdefMessage message = NdefMessage(rxNDEFMessagePtr, rxResult);
           NdefRecord record = message.getRecord(0);
           record.print();
           printer.write(record.getPayload(), record.getPayloadLength());
           
           for (int i = 0; i < 5; i++) {
             printer.write('\n');
           }
           delay(2500);
        }
        
        delay(500);
     } while(0);
     
     
}  
   

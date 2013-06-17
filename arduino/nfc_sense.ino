
#include <Streaming.h>

#include <Wire.h>
#include <SeeedGrayOLED.h>
#include <DHT.h>

#include <PN532.h>
#include <NFCLinkLayer.h>
#include <SNEP.h>
#include <NdefMessage.h>

#define SCK 13
#define MOSI 11
#define SS 10
#define MISO 12

#define DHT_PIN      7
#define DHT_TYPE     DHT22

#define LED_PIN     6
#define LED_ON()    digitalWrite(LED_PIN, HIGH)
#define LED_OFF()   digitalWrite(LED_PIN, LOW)
#define BUTTON_PIN  5

DHT dht(DHT_PIN, DHT_TYPE);

#define TEMPERATURE_N        (1 << 3)
#define TEMPERATURE_N_MASK   (TEMPERATURE_N - 1)
float lastTemperature = 0;
float temperatureBuffer[TEMPERATURE_N];
uint8_t theader = 0;
uint8_t ttial = 0;

char displayBuf[5 * TEMPERATURE_N + 1] = {0};

float lastHumidity = 0;
float humidityBuffer[TEMPERATURE_N];
uint8_t huheader = 0;


#define MAX_HEART_PULSE     2000
#define MAX_PULSE_N         (1 << 3)
#define MASK_PULSE_N        (MAX_PULSE_N - 1)

unsigned long lastTime = 0;
unsigned long heartBeatTime[MAX_PULSE_N] = {0};
uint8_t hheader = 0;
uint8_t htail  = 0;
float heartRate = -1;
uint8_t heartRateState = 0;
uint8_t lastHeartRateState = 1;

PN532 nfc(SCK, MISO, MOSI, SS);
NFCLinkLayer linkLayer(&nfc);
SNEP snep(&linkLayer);

// NDEF messages
#define MAX_PKT_HEADER_SIZE  50
#define MAX_PKT_PAYLOAD_SIZE 160
uint8_t txNDEFMessage[MAX_PKT_HEADER_SIZE + MAX_PKT_PAYLOAD_SIZE];
uint8_t *txNDEFMessagePtr; 
uint8_t txLen;

NdefRecord aarRecord = NdefRecord();

const uint8_t aarType[] = "android.com:pkg";
const uint8_t aarPayload[] = "com.seeedstudio.nfcdemo";

void setup() {
  Serial.begin(115200);
  Serial.println(F("----------------- NFC Sense --------------------"));
  
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT);
  
  Wire.begin();
  SeeedGrayOled.init();             //initialize SEEED OLED display
  SeeedGrayOled.clearDisplay();     //Clear Display.
  SeeedGrayOled.setNormalDisplay(); //Set Normal Display Mode
  SeeedGrayOled.setVerticalMode();  // Set to vertical mode for displaying text
  SeeedGrayOled.setGrayLevel(15);
  
  SeeedGrayOled.setTextXY(0, 0);
  SeeedGrayOled.putString("Temperature:");
  updateTemperature();
  
  for (int i = 0; i < TEMPERATURE_N; i++) {
    temperatureBuffer[i] = 0;
    humidityBuffer[i] = 0;
  }
    
  SeeedGrayOled.setTextXY(3, 0);
  SeeedGrayOled.putString("Humidity:");
  updateHumidity();
  
  SeeedGrayOled.setTextXY(6, 0);
  SeeedGrayOled.putString("Heart Rate:");
  updateHeartRate();
  
  attachInterrupt(1, heartBeat, RISING);
  
  txNDEFMessagePtr = &txNDEFMessage[MAX_PKT_HEADER_SIZE];
  aarRecord.setTnf(TNF_EXTERNAL_TYPE);
  aarRecord.setType(aarType, sizeof(aarType) - 1);
  aarRecord.setPayload(aarPayload, sizeof(aarPayload) - 1);

  nfc.initializeReader();
  nfc.SAMConfig();
}

void loop() {
   LED_ON();
   if (IS_ERROR(nfc.configurePeerAsTarget(SNEP_SERVER))) {
        extern uint8_t pn532_packetbuffer[];
        
        int8_t updateState = 0;
        while (!nfc.isReady()) {
          if (updateState == 0) {
            updateTemperature();
          } else if (updateState == 1) {
            updateHumidity();
          } else {
            updateHeartRate();
            updateState = -1;
          }
          updateState++;
        }

        nfc.readspicommand(PN532_TGINITASTARGET, (PN532_CMD_RESPONSE *)pn532_packetbuffer);
      }
      
      {
         NdefMessage message = NdefMessage();
         char *ptr = displayBuf;
         for (int i = 0; i < TEMPERATURE_N; i++) {
           dtostrf(temperatureBuffer[(theader + i) & TEMPERATURE_N_MASK], 4, 1, ptr);
           ptr[4] = ' ';
           ptr += 5;
         }
         ptr[-1] = '\0';
         message.addMimeMediaRecord("x/t", displayBuf);
#if 0         
         ptr = displayBuf;
         for (int i = 0; i < TEMPERATURE_N; i++) {
           dtostrf(humidityBuffer[(huheader + i) & TEMPERATURE_N_MASK], 4, 1, ptr);
           ptr[4] = ' ';
           ptr += 5;
         }
         ptr[-1] = '\0';
         message.addMimeMediaRecord("x/h", displayBuf);
#else
         message.addMimeMediaRecord("x/h", dtostrf(lastHumidity, 4, 1, displayBuf));
#endif
         message.addMimeMediaRecord("x/r", dtostrf(heartRate, 5, 1, displayBuf));
         message.addRecord(aarRecord);
         txLen = message.getEncodedSize();
         if (txLen <= MAX_PKT_PAYLOAD_SIZE) {
            message.encode(txNDEFMessagePtr);
          } else {
            Serial.println(F("Tx Buffer is too small."));
            while (1) {
            }
          }

        for (int i = 0; i < 3; i++) {
           if (snep.pushPayload(txNDEFMessagePtr, txLen) == RESULT_SUCCESS) {
             Serial.println(F("Succeed to push a NDEF message."));
             break;
           } else {
             Serial.println(F("Fail to push a NDEF message."));
           } 
         }
   }
   LED_OFF();
   
   unsigned long startTime = millis();
   while ((millis() - startTime) < 2000) {
     updateTemperature();
     updateHumidity();
     updateHeartRate();
   }
}

void updateTemperature(void) {
  float t = dht.readTemperature();
  
  if (!isnan(t)) {
    temperatureBuffer[theader & TEMPERATURE_N_MASK] = t;
    theader++;
  }
  
  if (lastTemperature == t) {
    return;
  }
  lastTemperature = t;
  SeeedGrayOled.setTextXY(1, 0);
  if (isnan(t)) {
     SeeedGrayOled.putString("-     ");
  } else {
    dtostrf(t, 4, 1, displayBuf);
    SeeedGrayOled.putString(displayBuf);
    SeeedGrayOled.putChar('C');
  }
}

void updateHumidity(void) {
  float h = dht.readHumidity();
  
  if (!isnan(h)) {
    humidityBuffer[theader & TEMPERATURE_N_MASK] = h;
    huheader++;
  }
  
  if (lastHumidity == h) {
    return;
  }
  lastHumidity = h;
  SeeedGrayOled.setTextXY(4, 0);
  if (isnan(h)) {
    SeeedGrayOled.putString("-     ");
  } else {
    dtostrf(h, 4, 1, displayBuf);
    SeeedGrayOled.putString(displayBuf);
    SeeedGrayOled.putChar('%');
  }
}

int buttonIsDown(void) {
  int result = 0;
  
  if (digitalRead(BUTTON_PIN) == HIGH) {
    result = 1;
  }
  return result;
}

void heartBeat(void) {
  unsigned long currentTime = millis();
  unsigned long delta = currentTime - lastTime;
  if (delta < 250) {
    return;  // anti-shake
  }
  
  lastTime = currentTime;
  if (delta > MAX_HEART_PULSE) {
    // 
    hheader = 0;
    htail = 0;
    heartRateState = 0;
    heartRate = -1;
    return;
  }
  
  heartBeatTime[hheader & MASK_PULSE_N] = currentTime;
  hheader++;
  if ((unsigned)(hheader - htail) > MAX_PULSE_N) {
    htail = hheader - MAX_PULSE_N;
    heartRate = ((float)60 * (MAX_PULSE_N - 1) * 1000) / (heartBeatTime[(hheader - 1) & MASK_PULSE_N] - heartBeatTime[htail & MASK_PULSE_N]);
    heartRateState = 1;
  }
}
 
void updateHeartRate(void) {
  if (heartRateState == 0 && lastHeartRateState != 0) {
    SeeedGrayOled.setTextXY(7, 0);
    SeeedGrayOled.putString("-     ");
  } else if (heartRateState == 1) {
    SeeedGrayOled.setTextXY(7, 0);
    dtostrf(heartRate, 5, 1, displayBuf);
    SeeedGrayOled.putString(displayBuf);
    heartRateState = 2;
  } else if (heartRateState == 2) {
    if ((millis() - lastTime) > MAX_HEART_PULSE) {
      heartRate = -1;
    }
  }
}

  

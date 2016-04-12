// NRF24L01+ pins
#define CE_PIN 9
#define CS_PIN 10

// ok reply
class O_payload {
  public:
  uint16_t replyTo;                 // 2 bytes
};

// measurement
class M_payload {
  public:
  uint8_t alarmSensorTypes[3] = {0};// 1 byte per cell = 3 bytes
  uint8_t tempSensorTypes[4] = {0}; // 1 byte per cell = 4 bytes
  uint16_t humidity[4] = {0};       // 2 bytes per cell = 8 bytes
  int16_t temp[4] = {0};            // 2 bytes per cell = 8 bytes
  uint16_t battLevel;               // 2 bytes
  uint8_t alarmLevel;               // 1 byte
  uint8_t sleepCycles;              // 1 byte
  bool alarms[3];                   // 1 byte per cell = 3 bytes
  uint16_t messageNumber;           // 2 bytes
};

// NRF24L01 setup
#define CHANNEL 100
#define NODE_ADDRESS 01

// NRF24L01
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// SLEEP AND IRQ
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

// Radio
RF24 radio(CE_PIN,CS_PIN);
RF24Network network(radio);

bool send_M(uint16_t to);

void handle_O(RF24NetworkHeader& header);

long interval = 1500;
long messageSent = 0;
uint16_t messageCount = 0;
void setup() {

Serial.begin(115200);

  SPI.begin();                                           // Bring up the RF network
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  network.begin(CHANNEL, NODE_ADDRESS );
}

void loop() {

  network.update();                                      // Pump the network regularly
  while ( network.available() )  {                      // Is there anything ready for us?
    Serial.print("Something for us: ");

    RF24NetworkHeader header;                            // If so, take a look at it
    network.peek(header);
    Serial.println((char)header.type);

    switch (header.type){                              // Dispatch the message to the correct handler.
      case 'O': handle_O(header); break;
      case 'S': break; //This is a sleep payload

      default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"),header.type);
                network.read(header,0,0);
                break;
    };
  }
  if(millis() - messageSent > interval) 
  {
    bool ok;

    ok = send_M(00); // send measurements to master
    if (ok){
      Serial.println("Send ok");
    }else{
      Serial.println("Send failed");
    }
  }
}

/**
 * Handle an 'O' message, ok response
 */
void handle_O(RF24NetworkHeader& header)
{
  Serial.println("Handle O");
  static O_payload message;
  network.read(header,&message,sizeof(message));
  Serial.print("ReplyTo: ");
  Serial.println(message.replyTo);
}

bool send_M(uint16_t to)
{
  Serial.println("Gonna send M");
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'M');

  M_payload message;
  message.messageNumber = messageCount++;
  messageSent = millis();
  return network.write(header,&message,sizeof(M_payload));
}




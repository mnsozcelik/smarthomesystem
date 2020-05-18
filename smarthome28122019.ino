#include "ThingSpeak.h"
#include "WiFiEsp.h"
#include "secrets.h"
#include "Servo.h"


char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
WiFiEspClient  client;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX(sarı), TX(mavi)
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif


// Counting channel details
unsigned long counterChannelNumber = SECRET_CH_ID_COUNTER;
const char * myCounterReadAPIKey = SECRET_READ_APIKEY_COUNTER;
unsigned int counterFieldNumber = 1; 
int in1=9;    //fan
int in2=10;   //servo1(kapı)
int in3=11;   //servo2(perde)
int in4=12;   //led

int srvard1=4; //beyaz
int srvard2=5; //yeşil 

bool relay1;
bool relay2;
bool relay3;
bool relay4;

long tmpcount=-2; // Geçici Count Değeri

Servo servo1;
Servo servo2;

void setup() {
  pinMode(in1,OUTPUT);// Fan Röle Pinmod
  pinMode(in2,OUTPUT);// Servo1 Röle Pinmod
  pinMode(in3,OUTPUT);// Servo2 Röle Pinmod
  pinMode(in4,OUTPUT);// Led Röle Pinmod
  
  servo1.attach(srvard1);//Servo 1
  servo2.attach(srvard2);//Servo 
  
  digitalWrite(in1,HIGH);// Röle 1 kapat
  digitalWrite(in2,HIGH);// Röle 2 kapat
  digitalWrite(in3,HIGH);// Röle 3 kapat
  digitalWrite(in4,HIGH);// Röle 4 kapat
  //Initialize serial and wait for port to open
  Serial.begin(115200);  

  // initialize serial for ESP module  
  setEspBaudRate(ESP_BAUDRATE);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  Serial.print("Searching for ESP8266..."); 
  
  // initialize ESP module
  WiFi.init(&Serial1);
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
    
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() 
{
  int statusCode = 0;
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      //delay(000);     
    } 
    Serial.println("\nConnected");
  }

  // Read in field 1 of the private channel which is a counter  
  long count = ThingSpeak.readLongField(counterChannelNumber, counterFieldNumber, myCounterReadAPIKey);
  
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Counter: " + String(count));
  }
  else
  {
    // If WiFi is not connected, we have redirected the incoming value to the default state.
    count = -1;
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }

  if(tmpcount!=count)
    {
        switch(count)
        {
        case 0:
          relay1=false;
          break;
        case 1:
          relay1=true;
          break;
        case 2:
          relay2=false;
          break;
        case 3:
          relay2=true;
          break;
        case 4:
          relay3=false;
          break;
        case 5:
          relay3=true;
          break;
        case 6:
          relay4=false;
          break;
        case 7:
          relay4=true;
          break;
        default:
          break;   
        }
        tmpcount= count;
        //I call the function for relay operations.
        operationRelays(count,relay1,relay2,relay3,relay4);
        delay(1000);
    }
}

//This function is for operations performed by the relay.
void operationRelays(int c,bool r1,bool r2,bool r3,bool r4){
  switch(c){
    case 0:
        digitalWrite(in1,HIGH); // Röle in1'i kapatıyor
        break;
    case 1:
        digitalWrite(in1,LOW); // Röle in1'i açıyor
        break;
    case 2:
        digitalWrite(in2,LOW); // Röle in2'yi açıyor
        servo1.write(0);
        delay(1000);
        digitalWrite(in2,HIGH); // Röle in2'yi kapatıyor
        break;
    case 3:
        digitalWrite(in2,LOW); // Röle in2'i açıyor
        servo1.write(90);
        delay(1000);
        digitalWrite(in2,HIGH); // Röle in2'i kapatıyor
        break;
    case 4: 
        digitalWrite(in3,LOW); // Röle in3'i açıyor
        servo2.write(0);
        delay(1000);
        digitalWrite(in3,HIGH); // Röle in3'i kapatıyor
        break;
    case 5:
        digitalWrite(in3,LOW); // Röle in3'i açıyor
        servo2.write(180);
        delay(1000);
        digitalWrite(in3,HIGH); // Röle in3'i kapatıyor
        break;
    case 6:
        digitalWrite(in4,HIGH); // Röle in4'i kapatıyor
        break;
    case 7:
        digitalWrite(in4,LOW); // Röle in4'i açıyor
        break;
  }
}

// This function attempts to set the ESP8266 baudrate. Boards with additional hardware serial ports
// can use 115200, otherwise software serial is limited to 19200.
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  Serial1.begin(baudrate);
}

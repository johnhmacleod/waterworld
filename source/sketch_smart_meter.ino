


/**  
 * Connect an ESP8266 with Flow Rate sensor to the IBM IoT Foundation  
 **/

#include <ESP8266WiFi.h>  
#include <PubSubClient.h>
#include <algorithm>

//-------- Customise these values with your WiFi Connectivity details----------- 
const char* ssid = "IBM Demo";
//const char * ssid = "IBMVISITOR";
const char* password = "access2016";  

//-------- Customise these 4 values with your device data from the IoT Platform -------- 
#define ORG "5729ox" 
#define DEVICE_TYPE "SMARTMETER" 

#define DEVICE_ID "SMART01"
#define TOKEN "SMART01TOKEN"

//#define DEVICE_ID "SMART02"
//#define TOKEN "U?NOS)NY50J8JlN!@f"


char server[] = ORG ".messaging.internetofthings.ibmcloud.com"; 
char topic[] = "iot-2/evt/status/fmt/json"; 
char authMethod[] = "use-token-auth"; 
char token[] = TOKEN; 
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;  

//Flow variables
volatile int  flow_frequency;  // Measures flow meter pulses
unsigned int  l_hour;          // Calculated litres/hour                      
unsigned char flowmeter = D1;  // Flow Meter Pin number
unsigned long currentTime;
unsigned long cloopTime;
unsigned long intervalTime;
float l_total = 0;


void flow ()                  // Interrupt function
{ 
   flow_frequency++;
} 

void readSensorData()    
{
   currentTime = millis();
   // Every second, calculate and print litres/hour
      intervalTime = currentTime - cloopTime;
      cloopTime = currentTime;    // Updates cloopTime
      Serial.print(intervalTime);
      Serial.println(" mSec");
      Serial.print(flow_frequency);
      Serial.println(" Hz");
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min. (Results in +/- 3% range)
      l_hour = (flow_frequency * (1000.0 / intervalTime) * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flow rate in L/hour 
      l_total += l_hour * (intervalTime/3600000.0);
      flow_frequency = 0;                   // Reset Counter
      Serial.print(l_hour, DEC);            // Print litres/hour
      Serial.println(" L/hour");
   }


void callback(char* topic, byte* payload, unsigned int length);

unsigned long previousMillis = 0; // will store last counter was read 
const long interval = 1000;     // interval at which to read sensor 

WiFiClient wifiClient; 

// Initialize PubSubClient
PubSubClient client(server, 1883, callback, wifiClient); 
 
void setup() {  
  Serial.begin(115200);   
  Serial.println();  
  Serial.print("Connecting to ");  
  Serial.print(ssid);  
  WiFi.begin(ssid,password); 
//  WiFi.begin(ssid); 
  while (WiFi.status() != WL_CONNECTED) {    
    delay(500);    
    Serial.print("."); 
  }   Serial.println("");  
  Serial.print("WiFi connected, IP address: ");  
  Serial.println(WiFi.localIP());    pinMode(flowmeter, INPUT);
  attachInterrupt(flowmeter, flow, RISING); // Setup Interrupt 
                                     // see http://arduino.cc/en/Reference/attachInterrupt
  sei();                            // Enable interrupts  
  currentTime = millis();
  cloopTime = currentTime;

}  


void loop() {  
  int x;  
  if (!!!client.connected()) {    
    Serial.print("Reconnecting client to ");    
    Serial.println(server);    
    while (!!!(client.connect(clientId, authMethod, token))) {  
      Serial.print(".");      
      delay(500);    
    }  
    Serial.println();  }   
    readSensorData(); // Fetch current flow
    if (l_hour == 0) {
      l_total = 0;
    }
    String payload = "{\"d\":{\"Flow\":" + String(l_hour) + ", \"ContTotal\":" + String(l_total) + "}}";  
    Serial.print("Sending payload: ");  
    Serial.println(payload);  
    if (client.publish(topic, (char*) payload.c_str())) {    
      Serial.println("Publish ok");  
    } else {  Serial.println("Publish failed");  }
      delay(5000);  
}  

// Callback routine if message arrives 
void callback(char* topic, byte* payload, unsigned int length) {  
  Serial.println("callback invoked"); 
} 


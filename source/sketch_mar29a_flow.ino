


/**  
 * Connect an ESP8266 with Flow Rate sensor to the IBM IoT Foundation  
 **/

#include <ESP8266WiFi.h>  
#include <PubSubClient.h>
#include <algorithm>

//-------- Customise these values with your WiFi Connectivity details----------- 
const char* ssid = "TEST";
//const char * ssid = "IBMVISITOR";
const char* password = "access2016";  

//-------- Customise these values with your device data from the IoT Platform -------- 
#define ORG "YOUR_ORG_HERE" 
#define DEVICE_TYPE "WATERFLOW" 
//#define DEVICE_ID "FLOW01" 
//#define TOKEN "FLOW01TOKEN" 
 
#define DEVICE_ID "FLOW02" 
#define TOKEN "FLOW02TOKEN" 

//#define DEVICE_ID "FLOW03"
//#define TOKEN "FLOW03TOKEN"


#define ArrayLength  10 // Length of averaging array

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

int flowArray[ArrayLength] = {0};

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
//  WiFi.begin(ssid,password); 
  WiFi.begin(ssid); 
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

int arrayIndex = 0;
int avg_l_hour;
int nonZeroReadingCount = 0;
double averagearray(int* arr, int number);

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
    if (l_hour > 0) {
      nonZeroReadingCount++;
      flowArray[arrayIndex++] = l_hour; //only create an entry in the average array if water is flowing!
    }
    avg_l_hour = (int) averagearray(flowArray, std::min(ArrayLength, nonZeroReadingCount));
    if(arrayIndex==ArrayLength) arrayIndex=0;
    String payload = "{\"d\":{\"Flow\":" + String(l_hour) + ", \"AvgFlow\":" + String(avg_l_hour) + "}}";  
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


double averagearray(int* arr, int number){
  int i;
  int max,min;
  double avg = 0;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    if (number > 0) avg = amount/number;  // We allow for empty arrays - just return 0
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}


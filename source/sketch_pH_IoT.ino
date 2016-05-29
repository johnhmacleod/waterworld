/*
 # This sample code is used to test the pH meter V1.0.
 # Editor : YouYou
 # Ver    : 1.0
 # Product: analog pH meter
 # SKU    : SEN0161
*/
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>


const char* ssid = "IBM Demo";
const char* password = "access2016";
void callback(char* topic, byte* payload, unsigned int length);
double avergearray(int* arr, int number);

//-------- Customise these 4 values with your device data from the IoT Platform -------- 
#define ORG "5729ox" 
#define DEVICE_TYPE "WATERPH" 

#define DEVICE_ID "PH0002" 
#define TOKEN "sj-Z!k9g(scA+5gC6c"

//#define DEVICE_ID "PH0001" 
//#define TOKEN "IOy2cSfy0p5HA(rxFK"
char server[] = ORG ".messaging.internetofthings.ibmcloud.com"; 
char topic[] = "iot-2/evt/status/fmt/json"; 
char authMethod[] = "use-token-auth"; 
char token[] = TOKEN; 
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;  

#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define sendInterval 800
#define ArrayLenth  40    //times of collection
#define Correction .9174 //2/2.18
int pHArray[ArrayLenth] = {0};   //Store the average value of the sensor feedback
int pHArrayIndex=0; 

WiFiClient wifiClient; 
// Initialize PubSubClient
PubSubClient client(server, 1883, callback, wifiClient); 

void setup(void)
{
 
 
  Serial.begin(115200);  
  Serial.println("pH meter experiment!");    //Test the serial monitor
  Serial.println();  
  Serial.print("Connecting to ");  
  Serial.print(ssid);  
  WiFi.begin(ssid , password); //With password
  // WiFi.begin(ssid);//No password
  while (WiFi.status() != WL_CONNECTED) {    
    delay(500);    
    Serial.print("."); 
  }
  Serial.println("");  
  Serial.print("WiFi connected, IP address: ");  
  Serial.println(WiFi.localIP()); 
}



void loop(void)
{
  static unsigned long samplingTime = millis();
  static unsigned long sendTime = millis();
  static float pHValue, voltage;
  
  if (millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth) pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024* Correction;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }
  if(millis() - sendTime > sendInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {  
  if (!!!client.connected()) {    
    Serial.print("Reconnecting client to ");    
    Serial.println(server);    
    while (!!!(client.connect(clientId, authMethod, token))) {  
      Serial.print(".");      
      delay(500);    
    }  
    Serial.println();  
   }

   char strpH[20];
   char strVoltage[20];
   dtostrf(pHValue, 0, 2, strpH);
   dtostrf(voltage, 0, 2, strVoltage);
   String payload = "{\"d\":{\"pH\":" + String(strpH) +
       ",\"Voltage\":" + String(strVoltage) + "}}";  
    Serial.print("Sending payload: ");  
    Serial.println(payload);  
    if (client.publish(topic, (char*) payload.c_str())) {    
      Serial.println("Publish ok");  
    } else {  Serial.println("Publish failed");  }
    delay(2000);    
    Serial.print("Voltage:");
    Serial.print(voltage,2);
    Serial.print("    pH value: ");
    Serial.println(pHValue,2);
    sendTime=millis();
  }
}
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
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

// Callback routine if message arrives 
void callback(char* topic, byte* payload, unsigned int length) {  
  Serial.println("callback invoked"); 
} 

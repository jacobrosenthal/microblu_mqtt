/*
 *                SSSSS  kk                            tt
 *               SS      kk  kk yy   yy nn nnn    eee  tt
 *                SSSSS  kkkkk  yy   yy nnn  nn ee   e tttt
 *                    SS kk kk   yyyyyy nn   nn eeeee  tt
 *                SSSSS  kk  kk      yy nn   nn  eeeee  tttt
 *                               yyyyy
 *
 * SkynetClient for http://skynet.im, OPEN COMMUNICATIONS NETWORK & API FOR
 * THE INTERNET OF THINGS.
 *
 * This sketch uses the MQTT library to connect to skynet. It sends a message
 * on successful connection and logs data every second.
 *
 * Requires the MQTT library from Nick O'Leary http://knolleary.net/arduino-client-for-mqtt/
 * And one modification in PubSubClient.h, increase MQTT_MAX_PACKET_SIZE from 128 to something like 256
 *
 * Works with ethernet shields compatible with EthernetClient library from
 * Arduino. If you don't know, grab the original
 * http://arduino.cc/en/Main/ArduinoWiFiShield
 *
 * Remember not to mess with wifis's unavailable pins (7, 10, 11, 12, 13 and 4 if using SD card)
 *
 * You will notice we're using F() in Serial.print. It is covered briefly on
 * the arduino print page but it means we can store our strings in program
 * memory instead of in ram.
 *
 */
 
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

char ssid[] = "yournetworkname";     //  your network SSID (name)
char pass[] = "yourpassword";  // your WPA network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

char server[] = "skynet.im";

//Your UUID and token for skynet.im TODO where to get one
char UUID[]  = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX";
char TOKEN[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

//another UUID well send a message to
char TOUUID[] = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"; 

WiFiClient client;
PubSubClient skynet(server, 1883, onMessage, client);

//we'll run this if anyone messages us
void onMessage(char* topic, byte* payload, unsigned int length) {

  // handle incoming messages, well just print it for now
  Serial.println(topic);
  for(int i =0; i<length; i++){
    Serial.print((char)payload[i]);
  }    
  Serial.println();
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");
}

void loop() {
  //we need to call loop for the mqtt library to do its thing and send/receive our messages
  if(skynet.loop()){
  
    //Lets log some data every time through the loop!
    //create a json string like "light":122
    //don't forget to escape those quotes!
    //here we grab whatever is hooked up to A0
    String messageString = "{\"light\":"+ String(int(analogRead(A0))) + "}";
    
    //housekeeping to turn our string into a character array
    char message[messageString.length()+1];
    messageString.toCharArray(message, messageString.length()+1);
  
    //Send away! The logging endpoint is called 'data'
    skynet.publish("data",message);
    
    //small delay here so we dont send too many mesages
    //but avoid delays and long running code, we need to let loop do its work!
    delay(1000);
  }else
  {
    //oops we're not connected yet or we lost connection
    Serial.println(F("connecting..."));

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);
  
      // wait 10 seconds for connection:
      delay(10000);
    }
      
    // skynet doesnt use client so send empty client string and YOUR UUID and token
    if (skynet.connect("", UUID, TOKEN)){

      //success!
      Serial.println(F("connected"));

      //you need to subscribe to your uuid to get messages for you
      skynet.subscribe(UUID);
      
      //Lets send a message! give the destination and payload in a json object
      //{"devices":"XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX","payload":"hi!"}
      //don't forget to escape those quotes!
      //also note the wifi shield has a character limit of like 80 so stay below that
      String messageString = "{\"devices\":\"" + String(TOUUID) + "\",\"payload\":\"hi!\"}" ;
  
      //housekeeping to turn our string into a character array
      char message[messageString.length()+1];
      messageString.toCharArray(message, messageString.length()+1);
    
      //Send away! The device to device endpoint is called 'message'
      skynet.publish("message",message);
    }
  } 
}

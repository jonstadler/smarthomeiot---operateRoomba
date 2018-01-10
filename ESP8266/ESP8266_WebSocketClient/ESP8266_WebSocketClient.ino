/*
 *Netmedias
 *
 *  Created on: 20.08.2015
 *  
 */
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>

// @@@@@@@@@@@@@@@ You only need to midify modify wi-fi and domain info @@@@@@@@@@@@@@@@@@@@
const char* ssid     = "yagayuga"; //enter your ssid/ wi-fi(case sensitiv) router name - 2.4 Ghz only
const char* password = "1815wnaranjaave";     // enter ssid password (case sensitiv)
char host[] = "smarthomeiot.herokuapp.com"; //enter your Heroku domain name like "espiot.herokuapp.com" 

int switchPin1 = 5; //light switch
const int relayPin1 = 3; //Garage door 1
int pingCount = 0;
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

int port = 80;
char path[] = "/ws"; 
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

DynamicJsonBuffer jsonBuffer;
String currState, oldState, message;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) { 


    switch(type) {
        case WStype_DISCONNECTED:
            //USE_SERIAL.printf("[WSc] Disconnected!\n");
           Serial.println("Disconnected! ");
            break;
            
        case WStype_CONNECTED:
            {
             Serial.println("Connected! ");
			    // send message to server when Connected
				    webSocket.sendTXT("Connected");
            }
            break;
            
        case WStype_TEXT:
            Serial.println("Got data");
       
            if (switchPin1 = 1){
              //Serial.print(distance);
              currState = "on";
              //Serial.println(currState);
            }else{
              //Serial.println(currState);
              currState = "off";
            }
            
            processWebScoketRequest((char*)payload);

            break;
            
        case WStype_BIN:

            hexdump(payload, length);
            Serial.print("Got bin");
            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}

void setup() {
    Serial.begin(115200);

    pinMode(switchPin1, INPUT); 
    
    Serial.setDebugOutput(true);
    
    pinMode(relayPin1, OUTPUT);

    
      for(uint8_t t = 4; t > 0; t--) {
          delay(1000);
      }
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    
    //Serial.println(ssid);
    WiFiMulti.addAP(ssid, password);

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
        delay(1000);
    }
    Serial.println("Connected to wi-fi");
    webSocket.begin(host, port, path);
    webSocket.onEvent(webSocketEvent);

}

void loop() {

    webSocket.loop();
        delay(100);
	// make sure after every 40 seconds send a ping to Heroku
	//so it does not terminate the websocket connection
	//This is to keep the conncetion alive between ESP and Heroku
	if (pingCount > 20) {
		pingCount = 0;
		webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
	}
	else {
		pingCount += 1;
	}
}


void processWebScoketRequest(String data){
            String jsonResponse = "{\"version\": \"1.0\",\"sessionAttributes\": {},\"response\": {\"outputSpeech\": {\"type\": \"PlainText\",\"text\": \"<text>\"},\"shouldEndSession\": true}}";
            JsonObject& req = jsonBuffer.parseObject(data);

            String instance = req["instance"];
            String state = req["state"];
            String query = req["query"];
            String message = "{\"event\": \"OK\"}";
            
            Serial.println("Data2-->"+data);
            Serial.println("State-->" + state);

            if(query == "?"){ //if command then execute
              Serial.println("Recieved query!");
                 if(currState=="on"){
                      message = "running";
                    }else{
                      message = "not running";
                    }
                   jsonResponse.replace("<text>", "Roomba " + instance + " is " + message );
                   webSocket.sendTXT(jsonResponse);
                   
            }else if(query == "cmd"){ //if query check state
              Serial.println("Recieved command!");
                   if(state != currState){
                         if(currState == "off"){
                            message = "running";
                          }else{
                            message = "not running";
                          }
                          digitalWrite(relayPin1, HIGH);
                          delay(1000);
                          digitalWrite(relayPin1, LOW);
                   }else{
                          if(currState == "off"){
                            message = "not currently running";
                          }else{
                            message = "already running";
                          }
                    }
                  jsonResponse.replace("<text>", "Roomba " + instance + " is " + message );
                  webSocket.sendTXT(jsonResponse);

            
            }else{//can not recognized the command
                    Serial.println("Command is not recognized!");
                   jsonResponse.replace("<text>", "Command is not recognized by garage door Alexa skill");
                   webSocket.sendTXT(jsonResponse);
            }
            Serial.print("Sending response back");
            Serial.println(jsonResponse);
                  // send message to server
                  webSocket.sendTXT(jsonResponse);
}




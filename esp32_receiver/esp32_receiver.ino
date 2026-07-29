/**
 * 温度接收网关
 * STM32 PA2 → ESP32 GPIO16, GND→GND, 115200
 */
#include <WiFi.h>
#include <WebServer.h>

const char* ssid="TempMonitor",*pwd="12345678";
WebServer server(80);
float tval=0;bool valid=false;unsigned long last=0;

void setup(){
    Serial.begin(115200);delay(500);
    Serial2.begin(115200,SERIAL_8N1,16,-1);
    Serial.println("\n=== TempMonitor Gateway ===");

    WiFi.softAP(ssid,pwd);
    server.on("/temp",[](){
        char j[64];snprintf(j,sizeof(j),valid?"{\"temperature\":%.1f}":"{\"temperature\":null}",tval);
        server.sendHeader("Access-Control-Allow-Origin","*");server.send(200,"application/json",j);
    });
    server.begin();
    Serial.printf("WiFi: %s\n",WiFi.softAPIP().toString().c_str());
}

void loop(){
    static String line;
    while(Serial2.available()){
        char c=Serial2.read();
        if(c=='\n'||c=='\r'){
            if(line.length()>0){
                int i=line.indexOf("T:");
                if(i>=0){tval=line.substring(i+2).toFloat();valid=true;last=millis();
                    Serial.printf("T:%.1f\n",tval);}
                line="";
            }
        }else{line+=c;if(line.length()>64)line="";}
    }
    if(valid&&millis()-last>5000)valid=false;
    server.handleClient();delay(5);
}

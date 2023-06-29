#ifndef _data_h_
#define _data_h_

#include <Arduino.h>
#include "wifi.h"
#include "memory.h"

///not for user. Chooses to send to wifi or pusher depending if its connected
void data_send(String payload){
    if(wifi_client.connected()){
        wifi_publish("/CVR2_data_to_server", payload);
    }
    else{
        Serial.println(payload);
    }
}

///Upload value through wifi or pusherboat
//Final message = "testrigs, testrig-name=Rig300X,client=Local_mitte_machine,board=2,probe=1 field=value
void uploadToGrafana(String field, double value){
    String payload = "\ntestrigs,testrig-name=Rig";
    payload += String(memoryRead(0)); //Add rig number (300X)
    payload += ",client=Local_mitte_machine,board=2,probe=1 ";
    payload += field;
    payload += "=";
    payload += String(value);
    payload += " ";
    data_send(payload);
}

#endif


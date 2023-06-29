//Put your wifi details in this file.
//In subscriptions, put the mqtt topic that you are listening for.
//In the messageReceived function, put what happens if that topic is detected.
//You can send mqtt messages with the publish function

#ifndef _wifi_h_
#define _wifi_h_

#include "_task_cfg.h"
#include "WiFiNINA.h"
#include "PubSubClient.h"
#include <Arduino.h>
#include "MQTT.h"
#include "memory.h"
#include "switch_cfg.h"
#include "global.h"

/// Connects to wifi and handles subscribes and publish to MQTT broker
void wifi(); 
void wifi_init(); 
void wifi_runnable(); 
void wifi_deinit();  

/******** Functions ********/
void wifi_messageReceived(String &topic, String &payload);
void wifi_retry();
void wifi_nodered();

///publish to mqtt with a topic and value.
void wifi_publish(const char topic[], const String &payload);
bool isDangerousTaskRunning();
/***************************/

/******** Variables ********/
#define BROKER_IP   "10.1.202.144"
const char* wifi_ssid     = "mitterigs";
const char* wifi_pass     = "$%23dontshare(H!#me";
WiFiClient wifi_net;
MQTTClient wifi_client;
unsigned long wifi_lastMillis = 0;

bool wifi_firstConnectSuccess = false;

bool paused = false;

bool startSamplerInit = false;
uint32_t startSamplerInitTime = 0;

String subscriptions[] = {
    "/CVR2_prime1_to_rig",
    "/CVR2_prime2_to_rig",
    "/CVR2_numDoses_to_rig",
    "/CVR2_prime3_to_rig",
    "/CVR2_doseTime_to_rig",

    "/CVR2_stagnationTime_to_rig",
    "/CVR2_stagnationTime_2_to_rig",
    "/CVR2_stagnationTime_3_to_rig",
    "/CVR2_numBottlesPerBatch_to_rig",
    "/CVR2_numBatches_to_rig",
    "/CVR2_timeBetweenBatches_to_rig",
    "/CVR2_timeBetweenBatches2_to_rig",
    "/CVR2_timeBetweenBatches3_to_rig",
    "/CVR2_bottleCounter_to_rig",

    "/CVR2_samplePeriod_to_rig",
    "/CVR2_sampleLength_to_rig",
    "/CVR2_sampling_to_rig",
    "/CVR2_samplerInit_to_rig",

    "/CVR2_pause_to_rig",
    "/CVR2_start_to_rig",
    "/CVR2_waterSensor_to_rig",
    "/CVR2_drain_to_rig",
    "/CVR2_state_to_rig",
    "/CVR2_CO2_to_rig",
    "/CVR2_ContPriming_to_rig",
    "/CVR2_serialPrintsEnabled_to_rig",
    "/CVR2_start_to_rig",
    "/CVR2_PWMadjust_to_rig",
    "/CVR2_testID_to_rig",
};

enum class WifiState
{
    OFF,
    WIFI_PENDING,
    BROKER_PENDING,
    CONNECTED
};

WifiState s_wifiState = WifiState::OFF;

/***************************/

void wifi_init(){
    analogWrite(PIN_BUZZER,LOW); //to stop alarm hangs
    print("\nStarting Wifi...",false);
    s_wifiState = WifiState::WIFI_PENDING;
}

static void brokerConnectionAttempt(){
    static char clientID[10];
    wifi_client.setOptions(12, false, 3000);
    wifi_client.begin(BROKER_IP, 1883, wifi_net);
    wifi_client.onMessage(wifi_messageReceived);
    String clientIDStr = "Rig" + String(memoryRead(0));
    clientIDStr.toCharArray(clientID,9);
    wifi_client.connect(clientID);
    s_wifiState = WifiState::BROKER_PENDING;
}

void wifi_runnable(){

    // connected to WIFI
    if (s_wifiState == WifiState::WIFI_PENDING){
        if(WiFi.status() == WL_CONNECTED){
            print("Wifi Started\n");
            print("Connecting to broker...");
            s_wifiState = WifiState::BROKER_PENDING;
        }
        else {            
            print("Attempt to connect to Wifi...");
            WiFi.end();
            WiFi.begin(wifi_ssid, wifi_pass);
        }
    }
    
    // connecting to broker
    else if (s_wifiState == WifiState::BROKER_PENDING){
        if(!wifi_client.connected()){
            if(WiFi.status() == WL_CONNECTED){
                print(".", false); //todo: timeout?
                brokerConnectionAttempt();
            }
            else
                wifi_retry();
        }
        else {
            print("Connected to broker");
            print("Subscribing...");
            int numSubs = sizeof(subscriptions) / sizeof(subscriptions[0]); //must be exact number
            bool success = false;
            for(int i = 0; i < numSubs; i++){
                if (!wifi_client.subscribe(subscriptions[i])){
                    break;
                }
                delay(10);
            }

            if (wifi_client.connected()){ //resets client connection if subscribing fails
                print("Connected!");
                wifi_firstConnectSuccess = true;
                s_wifiState = WifiState::CONNECTED;
            }
            else{
                print("Failed to Subscribe, retrying");
                // when subscription fails it disconnects from the broker
            }
        }
    }

    // running
    else if (s_wifiState == WifiState::CONNECTED){

        if(WiFi.status() != WL_CONNECTED || !wifi_client.connected()){
            wifi_retry();
        }

        else {
            wifi_client.loop();
            //update node red UI every 6 seconds
            if (millis() - wifi_lastMillis > 1000*6) {
                wifi_lastMillis = millis();
                wifi_nodered();
            }

            //finish init procedure x seconds after initializing
            if(startSamplerInit && millis() - startSamplerInitTime > 37000 && millis() - startSamplerInitTime < 38000){
                Serial1.write("G1\r");
                delay(100);
                Serial1.write("G1\r");
            }
            if(startSamplerInit && millis() - startSamplerInitTime > 40000){
                Serial1.write("GSp\r");
                delay(100);
                Serial1.write("GSp\r");
                startSamplerInit = false;
            }
        } 
    }
}

void wifi_deinit(){
    print("\nTurning off WiFi!");
    wifi_client.disconnect();
    WiFi.end();
    s_wifiState = WifiState::OFF;
}

void wifi_messageReceived(String &topic, String &payload) {
    int intkey = memoryRead(RIGNUM_EEPROM) % 3000; //get rig number to use as a key
    String key = String(intkey); //convert to string
    if(payload.charAt(0) == key.charAt(0)){ //reads rig number key in payload to see if the command is meant for this rig
        payload.remove(0,1); // removes the rig number key
        
        print("incoming: " + topic + " - " + payload);
        if (topic == "/CVR2_prime1_to_rig") {
            memoryWrite(PRIMING_EEPROM_TIMEOPEN1,payload.toInt());//Time first valve is open
            wifi_publish("/CVR2_prime1_to_server", String(memoryRead(PRIMING_EEPROM_TIMEOPEN1)));
        }
        if (topic == "/CVR2_prime2_to_rig") {
            memoryWrite(PRIMING_EEPROM_TIMECLOSE1,payload.toInt());//Time both valves are closed
            wifi_publish("/CVR2_prime2_to_server", String(memoryRead(PRIMING_EEPROM_TIMECLOSE1)));
        }
        if (topic == "/CVR2_prime3_to_rig") {
            memoryWrite(PRIMING_EEPROM_TIMEOPEN2,payload.toInt()); //Time second valve is open
            wifi_publish("/CVR2_prime3_to_server", String(memoryRead(PRIMING_EEPROM_TIMEOPEN2)));
        }
        if (topic == "/CVR2_numDoses_to_rig") {
            memoryWrite(PRIMING_EEPROM_NUMDOSES,payload.toInt()); //Number of doses
            wifi_publish("/CVR2_numDoses_to_server", String(memoryRead(PRIMING_EEPROM_NUMDOSES)));
        }
        if (topic == "/CVR2_doseTime_to_rig") {
            memoryWrite(PRIMING_EEPROM_DOSETIME,payload.toInt()); //Dosing length of pulse cycle
            wifi_publish("/CVR2_doseTime_to_server", String(memoryRead(PRIMING_EEPROM_DOSETIME)));
        }

        if (topic == "/CVR2_timeBetweenBatches_to_rig") { //Time between batches
            memoryWrite(PRIMING_EEPROM_TIMEBETWEENBATCHES,payload.toInt());
            memoryWrite(PRIMING_EEPROM_TIMEBETWEENBATCHES_2,payload.toInt());
            memoryWrite(PRIMING_EEPROM_TIMEBETWEENBATCHES_3,payload.toInt());
            wifi_publish("/CVR2_timeBetweenBatches_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES)));
            wifi_publish("/CVR2_timeBetweenBatches2_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_2)));
            wifi_publish("/CVR2_timeBetweenBatches3_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_3)));
        }
        if (topic == "/CVR2_timeBetweenBatches2_to_rig") { //Time between batches
            memoryWrite(PRIMING_EEPROM_TIMEBETWEENBATCHES_2,payload.toInt());
            wifi_publish("/CVR2_timeBetweenBatches2_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_2)));
        }
        if (topic == "/CVR2_timeBetweenBatches3_to_rig") { //Time between batches
            memoryWrite(PRIMING_EEPROM_TIMEBETWEENBATCHES_3,payload.toInt());
            wifi_publish("/CVR2_timeBetweenBatches3_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_3)));
        }
        if (topic == "/CVR2_stagnationTime_to_rig") {
            memoryWrite(PRIMING_EEPROM_stagnationTime,payload.toInt()); //Stagnation time
            memoryWrite(PRIMING_EEPROM_stagnationTime_2,payload.toInt()); //Stagnation time
            memoryWrite(PRIMING_EEPROM_stagnationTime_3,payload.toInt()); //Stagnation time
            wifi_publish("/CVR2_stagnationTime_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime)));
            wifi_publish("/CVR2_stagnationTime2_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime_2)));
            wifi_publish("/CVR2_stagnationTime3_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime_3)));
        }
        if (topic == "/CVR2_stagnationTime_2_to_rig") {
            memoryWrite(PRIMING_EEPROM_stagnationTime_2,payload.toInt()); //Stagnation time
            wifi_publish("/CVR2_stagnationTime2_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime_2)));
        }
        if (topic == "/CVR2_stagnationTime_3_to_rig") {
            memoryWrite(PRIMING_EEPROM_stagnationTime_3,payload.toInt()); //Stagnation time
            wifi_publish("/CVR2_stagnationTime3_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime_3)));
        }
        if (topic == "/CVR2_numBatches_to_rig") {
            memoryWrite(PRIMING_EEPROM_NUMBATCHES,payload.toInt()); //Number of batches
            wifi_publish("/CVR2_numBatches_to_server", String(memoryRead(PRIMING_EEPROM_NUMBATCHES)));
        }
        if (topic == "/CVR2_numBottlesPerBatch_to_rig") { //Number of bottles per batch
            memoryWrite(PRIMING_EEPROM_NUMBOTTLESPERBATCH,payload.toInt());
            wifi_publish("/CVR2_numBottlesPerBatch_to_server", String(memoryRead(PRIMING_EEPROM_NUMBOTTLESPERBATCH)));
        }

        if (topic == "/CVR2_samplePeriod_to_rig") {//Sampling Period
            memoryWrite(sampling_EEPROM_period,payload.toInt());
            wifi_publish("/CVR2_samplerPeriod_to_server", String(memoryRead(sampling_EEPROM_period)));
        }
        if (topic == "/CVR2_sampleLength_to_rig") { //Sampling length
            memoryWrite(sampling_EEPROM_length,payload.toInt());
            wifi_publish("/CVR2_samplerLength_to_server", String(memoryRead(sampling_EEPROM_length)));
        }

        if (topic == "/CVR2_sampling_to_rig") { //toggle CO2
            if(payload == "false") { memoryWrite(sampling_EEPROM_samplingEnabled,0);}
            else if (payload == "true"){ memoryWrite(sampling_EEPROM_samplingEnabled,1);}
            else if (payload == "sample"){ task_list[SAMPLING].start();}
            else if (payload == "prime"){ task_list[PRIMING].start();}
            else if (payload == "sensors"){ task_list[SENSORS].start();}
            else{
                Serial1.write("N\r");
                Serial.println(Serial1.readString());
            }
        }

        if (topic == "/CVR2_samplerInit_to_rig") { //init sampler 
            Serial1.write("I\r");
            delay(100);
            Serial1.write("I\r"); //try again
            startSamplerInit = true;
            startSamplerInitTime = millis();
        }

        if (topic == "/CVR2_pause_to_rig") { //Set rig to pause
            if(payload == "false") {paused = false;}
            else if(payload == "true"){paused = true;}
            wifi_publish("/CVR2_pause_to_server", String(paused));
            
        }
        if (topic == "/CVR2_start_to_rig") { //Start process (fillitk)
            task_list[FILLITK].start();
        }
        if (topic == "/CVR2_waterSensor_to_rig") {//Toggle if water sensor is enabled
            if(payload == "false"){ switch_list[SWITCH_CAPACITIVEWATERSENSOR].stop(); wifi_publish("/CVR2_waterSensor_to_server", "False");}
            else if (payload == "true"){ switch_list[SWITCH_CAPACITIVEWATERSENSOR].start(); wifi_publish("/CVR2_waterSensor_to_server", "True");}
        }
        if (topic == "/CVR2_drain_to_rig") {//draining task
            if(payload == "false") task_list[DRAINING].stop();
            else if (payload == "true") task_list[DRAINING].start();
            else memoryWrite(draining_EEPROM_drainLength, payload.toInt());
        }
        if (topic == "/CVR2_state_to_rig") { //clear error
            errorState(NOT_RUNNING);
            wifi_publish("/CVR2_state_to_server", String(errorState()));
        }
        if (topic == "/CVR2_bottleCounter_to_rig") { //clear bottle count
            memoryWrite(PRIMING_EEPROM_BOTTLECOUNTER, 0);
            memoryWrite(PRIMING_EEPROM_BATCHCOUNTER, 0);
            wifi_publish("/CVR2_batchCounter_to_server", String(memoryRead(PRIMING_EEPROM_BATCHCOUNTER)));
            wifi_publish("/CVR2_bottleCounter_to_server", String(memoryRead(PRIMING_EEPROM_BOTTLECOUNTER)));
        }

        if (topic == "/CVR2_CO2_to_rig") { //toggle CO2
            if(payload == "false") { memoryWrite(PRIMING_EEPROM_CO2ENABLED,0); wifi_publish("/CVR2_CO2_to_server", "0");}
            else if (payload == "true"){ memoryWrite(PRIMING_EEPROM_CO2ENABLED,1);wifi_publish("/CVR2_CO2_to_server", "1");}
        }

        if (topic == "/CVR2_ContPriming_to_rig") { //toggle Continues Priming
            if(payload == "false") { memoryWrite(PRIMING_EEPROM_CONTPRIMENABLED,0); wifi_publish("/CVR2_ContPriming_to_server", "0");}
            else if (payload == "true"){ memoryWrite(PRIMING_EEPROM_CONTPRIMENABLED,1);wifi_publish("/CVR2_ContPriming_to_server", "1");}
        }

        if (topic == "/CVR2_serialPrintsEnabled_to_rig") { //toggle Serial print
            if(payload == "false") { memoryWrite(system_EEPROM_serialPrintsEnabled,0); wifi_publish("/CVR2_serialPrintsEnabled_to_server", "False");}
            else if (payload == "true"){ memoryWrite(system_EEPROM_serialPrintsEnabled,1);wifi_publish("/CVR2_serialPrintsEnabled_to_server", "True");}
        }

        if (topic == "/CVR2_start_to_rig") { //Start rig
            task_list[FILLITK].start();
        }

        if (topic == "/CVR2_PWMadjust_to_rig") { //enable pwm adjust
            if(payload == "false"){memoryWrite(PRIMING_EEPROM_PWMADJUST,0);wifi_publish("/CVR2_PWMadjust_to_server", "0");}
            else if(payload == "true"){memoryWrite(PRIMING_EEPROM_PWMADJUST,1);wifi_publish("/CVR2_PWMadjust_to_server", "1");}
            else {memoryWrite(PRIMING_EEPROM_PWM,payload.toInt());wifi_publish("/CVR2_PWMadjust_to_server", String(memoryRead(PRIMING_EEPROM_PWM)));}
        }

        if (topic == "/CVR2_testID_to_rig") { //Sampling length
            memoryWrite(sampling_EEPROM_testID,payload.toInt());
            wifi_publish("/CVR2_testID_to_server", String(memoryRead(sampling_EEPROM_testID)));
        }
    }
}


///retry wifi if no dangerous tasks are running
void wifi_retry(){
    if(!isDangerousTaskRunning()){
        wifi_deinit();
        wifi_init();
    }
}

bool isDangerousTaskRunning(){
    for(int i = 0; i < MAX_TASKS; i++){
        if(task_list[i].state != STATE_STOPPED && !task_list[i].allowWifiReconnect){
            return true;
        }
    }
    return false;
}



///update node red UI with params
void wifi_nodered(){
    wifi_publish("/CVR2_tasksRunning", String(tasksRunning()));
    wifi_publish("/CVR2_connectionStatus_to_server", String(WiFi.RSSI()));
    wifi_publish("/CVR2_state_to_server", String(errorState()));
    wifi_publish("/CVR2_waterSensor_to_server", String(switch_list[SWITCH_CAPACITIVEWATERSENSOR].ticking_state));
    wifi_publish("/CVR2_bottleCounter_to_server", String(memoryRead(PRIMING_EEPROM_BOTTLECOUNTER)));
    wifi_publish("/CVR2_batchCounter_to_server", String(memoryRead(PRIMING_EEPROM_BATCHCOUNTER) + 1));
    wifi_publish("/CVR2_pause_to_server", String(paused));
    wifi_publish("/CVR2_prime1_to_server", String(memoryRead(PRIMING_EEPROM_TIMEOPEN1)));
    wifi_publish("/CVR2_prime2_to_server", String(memoryRead(PRIMING_EEPROM_TIMECLOSE1)));
    wifi_publish("/CVR2_prime3_to_server", String(memoryRead(PRIMING_EEPROM_TIMEOPEN2)));
    wifi_publish("/CVR2_numDoses_to_server", String(memoryRead(PRIMING_EEPROM_NUMDOSES)));
    wifi_publish("/CVR2_doseTime_to_server", String(memoryRead(PRIMING_EEPROM_DOSETIME)));
    wifi_publish("/CVR2_timeBetweenBatches_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES)));
    wifi_publish("/CVR2_timeBetweenBatches2_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_2)));
    wifi_publish("/CVR2_timeBetweenBatches3_to_server", String(memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_3)));
    wifi_publish("/CVR2_stagnationTime_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime)));
    wifi_publish("/CVR2_numBatches_to_server", String(memoryRead(PRIMING_EEPROM_NUMBATCHES)));
    wifi_publish("/CVR2_numBottlesPerBatch_to_server", String(memoryRead(PRIMING_EEPROM_NUMBOTTLESPERBATCH)));
    wifi_publish("/CVR2_stagnationTime_to_server", String(memoryRead(PRIMING_EEPROM_stagnationTime)));
    wifi_publish("/CVR2_stagnationTime_to_server2", String(memoryRead(PRIMING_EEPROM_stagnationTime_2)));
    wifi_publish("/CVR2_stagnationTime_to_server3", String(memoryRead(PRIMING_EEPROM_stagnationTime_3)));
    wifi_publish("/CVR2_samplerLength_to_server", String(memoryRead(sampling_EEPROM_length)));
    wifi_publish("/CVR2_samplerPeriod_to_server", String(memoryRead(sampling_EEPROM_period)));
    wifi_publish("/CVR2_CO2_to_server", String(memoryRead(PRIMING_EEPROM_CO2ENABLED)));
    wifi_publish("/CVR2_ContPriming_to_server", String(memoryRead(PRIMING_EEPROM_CONTPRIMENABLED)));
    wifi_publish("/CVR2_PWMadjust_to_server", String(memoryRead(PRIMING_EEPROM_PWMADJUST)));
    wifi_publish("/CVR2_PWMadjust_to_server", String(memoryRead(PRIMING_EEPROM_PWM)));
    wifi_publish("/CVR2_sampleEnabled_to_server", String(memoryRead(sampling_EEPROM_samplingEnabled)));
    wifi_publish("/CVR2_testID_to_server", String(memoryRead(sampling_EEPROM_testID)));
}

///publish to node red with the rig key attached 
void wifi_publish(const char topic[], const String &payload){
    String payloadWithKey = memoryRead(RIGNUM_EEPROM) % 3000 + payload; //adds rig key to start of payload
    wifi_client.publish(topic, payloadWithKey); //publishes
}

/**********************************/

void wifi(){

    task_enum i = WIFI;

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        wifi_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        wifi_runnable();
        break;
    
    case STATE_STOPPING:
        wifi_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


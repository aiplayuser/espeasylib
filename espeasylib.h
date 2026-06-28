#ifndef _ESPEASYLIB_H_
#define _ESPEASYLIB_H_
#include "Arduino.h"
#include "DNSServer.h" //自动弹窗
#include "ArduinoJson.h" //json解析
#include "AsyncMqttClient.h" //异步mqtt
#include "ESPAsyncWebServer.h" //异步Web
#include "LittleFS.h" //文件系统
#include "Ticker.h" //定时器
#ifdef ESP8266  
    #include "ESP8266WiFi.h" //wifi服务
    #define esps "esp8266" //esp类型
    #define gotip 3 //wifi状态
    inline uint32_t getfreefs() { //空闲闪存
        FSInfo fsinfo; LittleFS.info(fsinfo);
        return fsinfo.totalBytes - fsinfo.usedBytes;    }
#elif defined(ESP32)  
    #include <Update.h> //oat更新
    #include "WiFi.h" //wifi服务
    #define esps "esp32" //esp类型
    #define gotip 7 //wifi状态
    inline uint32_t getfreefs(){ //空闲闪存
        return LittleFS.totalBytes()-LittleFS.usedBytes(); }
#endif

File file; DNSServer dnssrv; WiFiClient wificlt; AsyncWebServer websrv(80);
AsyncMqttClient mqttclt; StaticJsonDocument<88> doc; char tmps[88]; Ticker ticker; 
String espid, ssid, pass, mqtts, topic, msgs, inijson="/ini.json";

const char htmlhead[] PROGMEM = R"( 
<!DOCTYPE html><html><head><meta charset=UTF-8>
<meta name=viewport content='width=device-width,initial-scale=1.2'></head><body><p>
<form action=/upload name=form1 method=POST enctype=multipart/form-data>
<input type=file name=data onchange=document.form1.submit()></form><p>
<form action=/delall method=POST onsubmit='return confirm()'>
<input type=submit value=Format> <a href=/scanwifi>搜索WiFi</a><p></form>
<form action=/again method=POST><input type=submit value=重启RST> 
<a href=/>返回home</a><p></form>            
)";

const char htmlwifi[] PROGMEM = R"( 
<form action=/joinwifi method=post>
<input name=ssid size=24 value={wifissid}>{wifirssi}<br>
<input name=pass size=15><input type=submit value=连接wifi></form><p>    
)";

const char jumppage[] PROGMEM = R"(
<h1>waiting...<script>setTimeout(()=>window.location='{locat}',999);</script>  
)";

const char htmlini[] PROGMEM = R"(
<form action=/saveini method=post>
mqtt <input name=mqtts size=20 value={mqtts}><br>
topic <input name=topic size=20 value={topic}><br>
SS ID <input name=ssid size=20 value={ssid}><br>
PASS <input name=pass size=20 value={pass}><p>
<input type=submit style=width:222px value=保存save></form><p>
freefs-{freefs}       
)";

const char htmlfile[] PROGMEM = R"(
<form action=/delone method=post onsubmit='return confirm()'>
<a href={fpath}>{fpath}</a>--{fsize}<br>
<input name=fpath size=17 value={fpath}>
<input type=submit value=删除Del></form>     
)";

void cjson(){ doc.clear(); file.close();}
void rjson(String path){ file=LittleFS.open(path,"r"); deserializeJson(doc,file); }
void wjson(String path){ file=LittleFS.open(path,"w"); serializeJson(doc,file); cjson(); }

String gettype(String path){ if(path.endsWith("json") ) return "text/json";
    if( path.endsWith("htm") || path.endsWith("html") ) return "text/html";
    return "application/octet-stream"; } //获取文件类型

void listen0(){ 
    if(!digitalRead(0)){ Serial.printf("\nreset\n"); delay(2222); 
        if(!digitalRead(0)){ LittleFS.remove(inijson); ESP.restart(); 
        } 
    }    
}  //监听D3重置

void sendmqtt(String str){ mqttclt.publish(topic.c_str(),0,false,str.c_str() ); }

void sendpage(AsyncWebServerRequest*req, String str ){
    req->send(200,"text/html;charset=utf-8",str); }
    
size_t otaSpace(){ return( ESP.getFreeSketchSpace() - 0x1000 ) & 0xFFFFF000; }

void homepage(AsyncWebServerRequest*req ){ String page=FPSTR(htmlhead);    
    page+="bindate: " + String(__TIME__) + " " + String(__DATE__) + "<br>"; 
    page+="espid-" + espid +"<br>"; page+="wifissid-" + WiFi.SSID() + "<br>";
    page+=(String)esps + "ip-" + WiFi.localIP().toString() + "<br>";
    page+=(String)esps + "ap-" + WiFi.softAPIP().toString() + "<br>"; 
    page+=(String)"flashsize-" + ESP.getFlashChipSize() + "<br>"; 
    page+=(String)"freeram-" + ESP.getFreeHeap() + "<p>";  
    page+=FPSTR(htmlini);
    page.replace("{mqtts}", mqtts+(mqttclt.connected()?"":"❌") );
    page.replace("{topic}", topic );
    page.replace("{ssid}", ssid );
    page.replace("{pass}", pass );
    page.replace("{freefs}", String(getfreefs()) );
    file = LittleFS.open("/", "r" ); 
    while(File entry = file.openNextFile()){ page+= FPSTR(htmlfile);
        page.replace("{fpath}", (String)"/" + entry.name() ); 
        page.replace("{fsize}", (String)entry.size() ); entry.close(); 
    } file.close(); sendpage(req, page);       
}

void setup1(){ 
    Serial.begin(9600); Serial.printf("\nbegin\nbegin\n"); 
    pinMode(0,INPUT_PULLUP); LittleFS.begin(); 
    espid=WiFi.macAddress(); listen0(); rjson(inijson); 
    ssid = doc["ssid"] | espid;  
    pass = doc["pass"] | ""; 
    topic = doc["topic"] | "administrator"; 
    mqtts = doc["mqtts"] | "broker.emqx.io"; cjson();

    WiFi.mode(WIFI_AP_STA); WiFi.softAP(ssid,pass); WiFi.begin(); WiFi.persistent(1);
    mqttclt.setServer(mqtts.c_str(),1883); mqttclt.setKeepAlive(60);
    mqttclt.onConnect([](bool){ mqttclt.subscribe(topic.c_str(),0); } );
    mqttclt.onDisconnect([](AsyncMqttClientDisconnectReason){ mqttclt.connect(); } );
    mqttclt.onMessage([](char*,char*pay,AsyncMqttClientMessageProperties,size_t len,size_t,size_t){
        msgs=""; for(int i=0;i<len;i++)msgs+=pay[i]; 
        if (msgs=="search..."){
            msgs="<a href=http://" + WiFi.localIP().toString() + " target=_blank>" + ssid + "</a>";
            sendmqtt(msgs);         } 
    } );     
    WiFi.onEvent([](WiFiEvent_t i){ 
        if(i!=7)Serial.printf("WiFiEvent: %d\n", i);
        if(i==gotip){Serial.println(WiFi.localIP()); mqttclt.connect(); } 
    } );

    websrv.begin(); dnssrv.start( 53, "*", WiFi.softAPIP() );

    websrv.onNotFound( [](AsyncWebServerRequest*req){ 
        String path=req->url(); 
        if(!LittleFS.exists(path)){ homepage(req); }
        else{ req->send(LittleFS, path, gettype(path)); } 
    } ); //处理所有请求 

    websrv.on("/", HTTP_GET, homepage);

    websrv.on("/again",[](AsyncWebServerRequest*req ){ 
        String page=FPSTR(htmlhead);
        page+=FPSTR(jumppage); page.replace("{locat}", "/" ); 
        sendpage(req, page); ticker.once(1,[](){ESP.restart();} );    
    } );

    websrv.on("/upload",HTTP_POST,
        [](AsyncWebServerRequest*req){
            Update.end(1)? req->redirect("/again"): req->redirect("/"); }, 
        [](AsyncWebServerRequest*,String name,size_t idx,uint8_t*data,size_t len,bool ok){
            bool isbin = name.endsWith(".bin");
            #ifdef ESP8266
                Update.runAsync(true);
            #endif
            if(isbin){ 
                Update.begin(otaSpace(),U_FLASH); Update.write(data,len); 
            }else{ 
                if(!idx)file=LittleFS.open("/"+name,"w"); 
                file.write(data,len); if(ok)file.close();   
            } 
        } 
    ); 

    websrv.on("/saveini",[](AsyncWebServerRequest *req){ 
        ssid=req->arg("ssid"); pass=req->arg("pass"); 
        topic=req->arg("topic"); mqtts=req->arg("mqtts"); 
        rjson(inijson); doc["ssid"]=ssid; doc["pass"]=pass; 
        doc["topic"]=topic; doc["mqtts"]=mqtts; 
        wjson(inijson); WiFi.softAP(ssid,pass); req->redirect("/"); 
    } ); //保存config

    websrv.on("/joinwifi",[](AsyncWebServerRequest*req){ homepage(req); 
        WiFi.begin(req->arg("ssid"),req->arg("pass")); 
    } );

    websrv.on("/scanwifi",[](AsyncWebServerRequest *req){
        String page=FPSTR(htmlhead); 
        int n = WiFi.scanComplete();
        if(n>0){ 
            for(int i=0;i>-88;i--){ 
                for(int j=0;j<n;j++){ 
                    if(WiFi.RSSI(j)==i){ page += FPSTR(htmlwifi); 
                        page.replace("{wifissid}", WiFi.SSID(j) ); 
                        page.replace("{wifirssi}", (String)WiFi.RSSI(j) ); 
            } } } sendpage(req,page); WiFi.scanDelete();
        }else{ 
            WiFi.scanNetworks(true); page+=FPSTR(jumppage); 
            page.replace("{locat}", "/scanwifi" ); 
            sendpage(req,page); 
        }             
    } ); //扫描wifi
                           
    websrv.on("/delone",[](AsyncWebServerRequest *req){ 
        LittleFS.remove(req->arg("fpath")); req->redirect("/"); 
    } );
    websrv.on("/delall",[](AsyncWebServerRequest *req){ 
        LittleFS.format(); req->redirect("/"); 
    } );  

}

void loop1(){ listen0(); dnssrv.processNextRequest(); }
 

#endif


    

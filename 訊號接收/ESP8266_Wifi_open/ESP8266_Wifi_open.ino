#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
const char* host = "open";
const char* ssid = "open";
const char* passphrase = "12345678";
String content,st;
int statusCode,Wifimode,modeCode;
ESP8266WebServer server(80);

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  EEPROM.begin(512);
  delay(3000);
  String essid;
  for (int i = 0; i < 32; ++i) 
    {     
      essid += char(EEPROM.read(i));
    }
  char charBuf[32];
  essid.toCharArray(charBuf, 32);
  String epassword="";
  for (int i = 32; i < 96; ++i) 
    {     
      epassword += char(EEPROM.read(i));
    }
  WiFi.begin(essid.c_str(), epassword.c_str());
  if (testWifi()) 
    {
      launchWeb(0);
      
    }
    else
    {
      SetupToAPMode(); //沒連上時執行這個
    }
}
void loop() {
  server.handleClient();
  }
  
  bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect...");
  //delay 10秒讓wifi連線
  while ( c < 10 ) {
    if (WiFi.status() == WL_CONNECTED)
    { 
      return true; 
    } //return一執行就結束testwifi這個函數, 有連上就return true, 沒連上就return false
    delay(1000);
    Serial.print(WiFi.status());
    c++;
  }
  return false;
}



void launchWeb(int webtype) {
  Serial.println("");
  if(webtype) {
    Serial.println("Operation in AP Mode.");
  }
  else {
    Serial.println("WiFi connected.");
  }
  createWebServer(webtype);
  // Start the server
  server.begin();
}

void SetupToAPMode(void) {
  ScanNetwork();
  WiFi.softAP(ssid, passphrase, 6,0);
  Serial.println("Soft AP Starting...");
  launchWeb(1);
  Serial.println("The End.");
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {  //ap mode
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        content = "<!DOCTYPE HTML><html><title>雲端無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>     你好, 請輸入您家裡的無線網路設定</p>";
        content += st;
        //&nbsp;代表塞一個空白, 重覆5個就是塞5個空白     
        content += "<br>";
        content += "<form method='get' action='setting'>";
        content += "<table border=\"0\"><tr><td><label>SSID</label></td><td><input type=\"text\" placeholder=\"請輸入要連接的SSID\" name='ssid' maxlength=32 size=64></td></tr>";
        content += "<tr><td><label>PASSWORD</label></td><td><input type=\"text\" placeholder=\"請輸入要連接的密碼\" name='pass' maxlength=64 size=64></td></tr></table>";
        content += "<input type=\"button\" value=\"重新掃瞄無線網路\" onclick=\"self.location.href='/rescannetwork'\">";
        content += "&nbsp;&nbsp;&nbsp;<input type='reset' value=\"重設\">&nbsp;&nbsp;&nbsp;<input type='submit' value=\"儲存\"></form></html>";
        server.send(200, "text/html", content);  //200代表伺服器回應OK, text/html代表用html網頁類型, 不加這個會找不到網頁
      });   

    server.on("/rescannetwork", []() {       
      content = "<!DOCTYPE HTML><html><title>雲端無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>重新掃瞄無線網路中...</p>";
      content += "<Meta http-equiv=\"ReFresh\" Content=\"3; URL='/'\">"; //讓網頁5秒自動跳回root page
      server.send(200, "text/html", content);
      modeCode = 0;
      ScanNetwork();
      });

  server.on("/setting", []() {
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    if (qsid.length() > 0 && qpass.length() > 0)
    {
      for (int i = 0; i < 96; ++i) 
      { 
        EEPROM.write(i, 0);
      }        
      for (int i = 0; i < qsid.length(); ++i)
        {
        EEPROM.write(i, qsid[i]);
        }
      for (int i = 0; i < qpass.length(); ++i)
        {
        EEPROM.write(32+i, qpass[i]);
        }   
      EEPROM.commit(); //EEPROM.write並不會馬上寫入EEPROM, 而是要執行EEPROM.commit()才會實際的寫入EEPROM
      content = "<!DOCTYPE HTML><html><title>重啟無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>儲存成功, 將自動重新開機!</p>";
      content += "<Meta http-equiv=\"ReFresh\" Content=\"3; URL='/RESTEST'\">";
      content +="<html>";
      statusCode = 200;
      }
    else {         
      content = "<!DOCTYPE HTML><html><title>輸入錯誤</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>輸入錯誤!!!SSID或PASSWORD任何其中一欄不能是空白, 按上一頁重新輸入</p>";
      content += "<input type=\"button\" value=\"上一頁\" onclick=\"self.location.href='/'\"></html>";
      //content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      Serial.println("Sending 404");   
      }   
    server.send(statusCode, "text/html", content);     
    });
    server.on("/RESTEST", []() {ESP.restart();});
  }
  else if (webtype == 0) {  //sta mode
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
       content = "<!DOCTYPE HTML><html><title>雲端控制網頁</title>";
      content += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
      content += "<p>你好, 請按下面的按鈕來開關，1秒內鎖上</p>";
      content += "<input style=\"width:120px;height:40px;font-size:20px;\" type=\"button\" id = \"button_開鎖\" value=\"開鎖\"><br>";
      content +="<script  language=\"JavaScript\">";
      content +="document.getElementById('button_開鎖').addEventListener(\"click\",button_開關); \r\n";
      content +="function button_開關(){  \r\n";
      content +="document.getElementById('button_開鎖').disabled=true; \r\n";
      content +="var oAjaxon=new XMLHttpRequest(); \r\n";
      content +="oAjaxon.open('GET','/open',true); \r\n";
      content +="oAjaxon.send(); \r\n";
      content += "oAjaxon.onreadystatechange = function button_開關() { \r\n";
      content +="if (oAjaxon.readyState == 4&&oAjaxon.status == 200){ } \r\n";
      content +="} \r\n";
      content += "setTimeout(\"document.getElementById('button_開鎖').disabled=false\",1000);} \r\n";
      content +="</script>";
      content += "<br><br>";  //空一行
      content += "<p>若要清除無線網路設定, 請按下方的清除無線網路設定按鈕</p>";
      content += "<input type=\"button\" value=\"清除無線網路設定\" onclick=\"self.location.href='/cleareeprom'\"></html>";    
      server.send(200, "text/html", content);
      });
  
    server.on("/open", []() {
      server.send(200, "text/html", content);
      digitalWrite(2,LOW);
      delay(1000);
      digitalWrite(2,HIGH);
      });
    server.on("/cleareeprom", []() {                             
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
      content += "<p>已清除無線網路設定</p></html>";
      server.send(200, "text/html", content);
      //Serial.println("clearing eeprom");
      });
    }
}

void ScanNetwork() {
    WiFi.disconnect(); //中斷聯線
    WiFi.mode(WIFI_AP);
    delay(100);
    int n = WiFi.scanNetworks(); //搜尋鄰近的無線網路。回傳搜尋到的無線網路數量
    Serial.print("Scan Network Done...and ");
    if (n == 0)
      Serial.println("No Any Networks Found!"); //搜尋不到網路
    else
      {
      Serial.print(n);
      Serial.println(" Networks Found!"); //搜尋到網路
      for (int i = 0; i < n; ++i)
        {
  //開始輸出所搜尋到的WIFI
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i)); //搜尋到的SSID
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i)); //搜尋到的wifi訊號
        Serial.print(")");
        byte encryption = WiFi.encryptionType(i); //取地無線網路的加密方法
        Serial.print("加密類型:");
        //WiFi.encryptionType(i)回傳的值 TKIP (WPA) = 2 , WEP = 5 , CCMP (WPA) = 4 , NONE = 7 , AUTO = 8(WPA or WPA2)
        switch (encryption) {
          case 2: Serial.println("TKIP(WPA)");break;
          case 5: Serial.println("WEP");break;
          case 4: Serial.println("CCMP(WPA)");break;
          case 7: Serial.println("NONE");break;
          case 8: Serial.println("AUTO(WPA or WPA2)");break;
          }     
        //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
        delay(100);
      }
    }
  Serial.println("");
  String k;
  st = "<ol type=\"1\" start=\"1\">";
  for (int i = 0; i < n; ++i)
    {
    //網頁顯示所搜尋到的WIFI
    st += "<table border=\"0\"><tr><td width=\"300px\">";
    k=String(i+1);
    st += k + ". ";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += "</td><td width=\"200px\">";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
    byte encryption = WiFi.encryptionType(i);
    switch (encryption) {
      case 2: st += "TKIP(WPA)";break;
      case 5: st += "WEP";break;
      case 4: st +="CCMP(WPA)";break;
      case 7: st +="NONE";break;
      case 8: st +="AUTO(WPA or WPA2)";break;
      } 
    //st += "</li></td></tr>"; 
    st += "</td></tr>";   
    }
  st += "</ol></table><br>";
  }

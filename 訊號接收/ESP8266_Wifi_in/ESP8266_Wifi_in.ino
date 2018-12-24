#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <DHT.h>
#include <FS.h>
#define MAX_SRV_CLIENTS 4
DHT dht;
const char* host = "esp8266";
const char* ssid = "jeff_wifi";
const char* passphrase = "12345678";
String content,st, Temandhum;
int statusCode,Wifimode,modeCode,delaytemper= 0;
float temperature, humidity;
String clinename[MAX_SRV_CLIENTS];
ESP8266WebServer webserver(80);
WiFiServer server(53333);
IPAddress aplocalip(192,168,0,1), gatIP(192,168,0,1), subnetMaskIP(255,255,255,0);
WiFiClient serverclient;  
void rootRouter() {
  File file = SPIFFS.open("/index.html", "r");
  webserver.streamFile(file, "text/html");
  file.close();
}
void MSND_out(){
  if (!MDNS.begin("moon")) {
    MDNS.addService("http", "tcp", 80); 
  }  
}
void closeusername() {
	for (size_t i = 0; i >= MAX_SRV_CLIENTS; i++)
	{
		clinename[i] = "";
	}
}
void setup() {
	closeusername();
  dht.setup(D1); // data pin 2
  server.begin(53333); 
  SPIFFS.begin();
  webserver.begin(80);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(aplocalip, gatIP, subnetMaskIP);
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

	void loop() {                                      //<--------------------------------------------------------------loop啊!!!!!	
		//MSND_out();
		serverclient = server.available();
		if (serverclient)
		{
			ConnectionExamination();
		}

		webserver.handleClient();
	}



void ConnectionExamination() {  //連線機制檢查
		String clientinstring = serverclient.readString(),test;
		if (clientinstring.length() >= 4 && clinename->indexOf(clientinstring) < 0)
		{
		for (size_t i = 0; i < MAX_SRV_CLIENTS; i++)
		{
			if (clinename[i] == NULL && clientinstring.indexOf("bear") >= 0)
			{
				Serial.println(clinename[i]);
				String to_string = String(i);
				clinename[i] = to_string + ".";
				clinename[i] += clientinstring;
                test =clinename[i];
				Serial.println(test);
				break;
			}
			else if (clinename[4] != clientinstring)
			{
				Serial.println("連線數已滿");
			}
     
		}	
		}
		
}

void serverclientintext(String userclientNumber,String clientouttext){ //文字輸出給客戶端
  serverclient = server.available();
	String textclientname = userclientNumber + "_" + clientouttext;
	serverclient.println(textclientname);
	
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
	
  createWebServer(webtype);
  webserver.begin();
}

void SetupToAPMode(void) {
  ScanNetwork();
  WiFi.softAP(ssid, passphrase, 6,0);
  launchWeb(1);
  
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {  //ap mode
    webserver.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        content = "<!DOCTYPE HTML><html><title>雲端電燈的無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>     你好, 請輸入您家裡的無線網路設定</p>";
        content += st;
        //代表塞一個空白, 重覆5個就是塞5個空白     
        content += "<br>";
        content += "<form method='get' action='setting'>";
        content += "<table border=\"0\"><tr><td><label>SSID</label></td><td><input type=\"text\" placeholder=\"請輸入要連接的SSID\" name='ssid' maxlength=32 size=64></td></tr>";
        content += "<tr><td><label>PASSWORD</label></td><td><input type=\"text\" placeholder=\"請輸入要連接的密碼\" name='pass' maxlength=64 size=64></td></tr></table>";
        content += "<input type=\"button\" value=\"重新掃瞄無線網路\" onclick=\"self.location.href='/rescannetwork'\">";
        content += "&nbsp;&nbsp;&nbsp;<input type='reset' value=\"重設\">&nbsp;&nbsp;&nbsp;<input type='submit' value=\"儲存\"></form></html>";
        webserver.send(200, "text/html", content);  //200代表伺服器回應OK, text/html代表用html網頁類型, 不加這個會找不到網頁
      });   

    webserver.on("/rescannetwork", []() {       
      content = "<!DOCTYPE HTML><html><title>雲端電燈的無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>重新掃瞄無線網路中...</p>";
      content += "<Meta http-equiv=\"ReFresh\" Content=\"3; URL='/'\">"; //讓網頁5秒自動跳回root page
      webserver.send(200, "text/html", content);
      modeCode = 0;
      ScanNetwork();
      });

  webserver.on("/setting", []() {
    String qsid = webserver.arg("ssid");
    String qpass = webserver.arg("pass");
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
    webserver.send(statusCode, "text/html", content);     
    });
  webserver.on("/RESTEST", []() {  ESP.reset(); });
  }
  else if (webtype == 0) {  //sta mode        <------------------------------------------------------進入顯示網頁
	  
    webserver.on("/", rootRouter);
    webserver.on("/index.html", rootRouter);
    webserver.on("/cleareeprom", []() {                             
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
      content += "<p>已清除無線網路設定</p></html>";
      webserver.send(200, "text/html", content);
      //Serial.println("clearing eeprom");
      WiFi.disconnect();
	  ESP.reset();
      });
	webserver.on("/on", []() {
	  serverclientintext("bear_power_switch", "on");
		webserver.send(200, "text/html", content);
		});
	webserver.on("/off", []() {
	  serverclientintext("bear_power_switch", "off");
		webserver.send(200, "text/html", content);
	});
	webserver.on("/useclient", []() {
		String temptext = "",temp;
		for(String temp:clinename)
		{
			temptext += temp+"\r\n";
		}
		content = "<!DOCTYPE HTML>\r\n<html>";
		content += "<body>";
		content += temptext;
		content += "<br>";
		content += "</body>";
		content += "</html>";
		webserver.send(200, "text/html", content);
	});
 webserver.on("/date", []() {
  delay(dht.getMinimumSamplingPeriod());
  humidity = dht.getHumidity();         //讀取濕度
  temperature = dht.getTemperature();      //讀取攝氏溫度
  Temandhum = "溫度" + String((float)temperature) + "濕度" + String((float)humidity);
  Serial.println(Temandhum);
	 content = "<!DOCTYPE HTML>\r\n<html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
	 content += "<body>";
	 content += Temandhum;
	 content += "<br>";
	 content += "</body>";
	 content += "</html>";
    webserver.send(200, "text/html", content);
  });
    }
}

void ScanNetwork() {
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
    st += "</td></tr>";   
    }
  st += "</ol></table><br>";
  }

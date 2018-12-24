#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
const char* host = "bear_power_switch";
const char* ssid = "bear_power_switch";
const char* passphrase = "12345678";
String content, st, inservertext;
int statusCode, Wifimode, serverport =53333;
ESP8266WebServer server(80);
WiFiClient userclient;
IPAddress inlocalip(192,168,0,1);
void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  EEPROM.begin(512);
  Serial.println("運轉中......");
  String essid;
  for (int i = 0; i < 32; ++i)
  {
    essid += char(EEPROM.read(i));
  }
  char charBuf[32];
  essid.toCharArray(charBuf, 32);
  String epassword = "";
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
    SetupToAPMode(); 
  }
}
void loop() {    //<--------------------------------------------------------------------------------loop
	if ((userclient.connect(inlocalip, serverport)) && (userclient.available() >= 0))
	{
		receivetext();
	}
 
 server.handleClient();
}
void receivetext() {           //接收伺服器端的文字
		inservertext = userclient.readString();
		Serial.println(inservertext);		
		if (inservertext.indexOf(host) >= 0)
		{
			if (inservertext.indexOf("on") >= 0)
			{
				digitalWrite(2, HIGH);
			}
			else if (inservertext.indexOf("off") >= 0)
			{
				digitalWrite(2, LOW);
			}
		}
	
}
bool clientline(void) {  //檢查是否連上伺服器端
	int a = 0;
	while (a < 2)
	{
		if (userclient.connect(inlocalip, serverport))
		{
			Serial.println("連結成功");
			return true;
		}
		delay(1000);
		a++;
	}
	return false;
}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect...");
  while ( c < 5 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    } //return一執行就結束testwifi這個函數, 有連上就return true, 沒連上就return false
    delay(1000);
    Serial.println(WiFi.status());
    c++;
  }
  return false;
}



void launchWeb(int webtype) {
  Serial.println("");
  createWebServer(webtype);
  server.begin();
}

void SetupToAPMode(void) {
  Serial.println("檢查點SetupToAPMode 進入");
  ScanNetwork();
  WiFi.softAP(ssid, passphrase, 6, 0);
  launchWeb(1);
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {  //ap mode
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      content = "<!DOCTYPE HTML><html><title>雲端電燈的無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>     你好, 請輸入您家裡的無線網路設定</p>";
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
      content = "<!DOCTYPE HTML><html><title>雲端電燈的無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>重新掃瞄無線網路中...</p>";
      content += "<Meta http-equiv=\"ReFresh\" Content=\"5; URL='/'\">"; //讓網頁5秒自動跳回root page
      server.send(200, "text/html", content);
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
          EEPROM.write(32 + i, qpass[i]);
        }
        EEPROM.commit(); //EEPROM.write並不會馬上寫入EEPROM, 而是要執行EEPROM.commit()才會實際的寫入EEPROM
        content = "<!DOCTYPE HTML><html><title>重啟無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>儲存成功, 將自動重新開機!</p>";
        content += "<Meta http-equiv=\"ReFresh\" Content=\"3; URL='/RESTEST'\">";
        content += "<html>";
        statusCode = 200;
        //WiFi.mode(WIFI_STA);
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

    server.on("/RESTEST", []() {
      ESP.reset();
    });
  }
  else if (webtype == 0) {  //sta mode
	  while (!clientline())
		  {
		   userclient.connect(inlocalip, serverport);
			delay(3000);

			if (clientline())
			{
				
				break;
				
			}
		  }
	  userclient.println(host);
     Serial.println("連接上跳出迴圈");
	// WiFi.mode(WIFI_STA);

  }
}

void ScanNetwork() {
  //WiFi.mode(WIFI_AP); //切換STA模式
  Serial.println("ScanNetwork進入 ");
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
        case 2: Serial.println("TKIP(WPA)"); break;
        case 5: Serial.println("WEP"); break;
        case 4: Serial.println("CCMP(WPA)"); break;
        case 7: Serial.println("NONE"); break;
        case 8: Serial.println("AUTO(WPA or WPA2)"); break;
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
    k = String(i + 1);
    st += k + ". ";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += "</td><td width=\"200px\">";
    byte encryption = WiFi.encryptionType(i);
    switch (encryption) {
      case 2: st += "TKIP(WPA)"; break;
      case 5: st += "WEP"; break;
      case 4: st += "CCMP(WPA)"; break;
      case 7: st += "NONE"; break;
      case 8: st += "AUTO(WPA or WPA2)"; break;
  }
    st += "</td></tr>";
  }
  st += "</ol></table><br>";
}


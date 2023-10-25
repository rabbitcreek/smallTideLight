#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
long hour;
long minute;
int hourCurrent;
int minuteCurrent;
bool fLow;
int failConnect = 1;
#define CONFIG_NOAA_STATION "9455920"
#define CONFIG_USER_AGENT "someone@example.com"
#define CONFIG_OFFSET_FROM_UTC (-9)
unsigned long weatherTimer;
const int CCH = 128;
String jsonBuffer;
char rgch[CCH];
int timerLight = 0;
int timeIndex = 1;
int lightLevel = 20;





WiFiClientSecure client;


bool ConnectToWiFi(void)
{
  const char *ssid = "werner";
  const char *password = "9073456071";
  int count = 0;

  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
    if (++count > 60)
      return false;
  }
  Serial.println("\nWiFi connected");
  return true;
}




void ConnectToNoaa(
    WiFiClientSecure &client,
    int &hour,
    int &minute)
{
  const char *host = "api.tidesandcurrents.noaa.gov";
  const int httpsPort = 443;
  static const char noaa_rootCA_cert[] PROGMEM =
      "-----BEGIN CERTIFICATE-----\n"
      "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
      "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
      "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
      "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
      "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
      "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
      "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
      "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
      "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
      "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
      "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
      "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
      "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
      "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
      "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
      "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
      "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
      "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
      "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
      "-----END CERTIFICATE-----\n";

  //client.setCACert(noaa_rootCA_cert);
  client.setInsecure();
  delay(500);
  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    failConnect = 1;
    return;
  }
  failConnect = 5;
  
  
  const char url[] = "/api/prod/datagetter?product=predictions&application=Custom&date=recent&datum=MLLW&station=9455920&time_zone=lst_ldt&units=english&interval=hilo&format=csv";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: " CONFIG_USER_AGENT "\r\n" +
               "Connection: close\r\n\r\n");
  /* 
    Returned text looks like:
    Date Time, Prediction, Type
    2018-12-25 00:54,-3.304,L
    2018-12-25 08:06,16.223,H
    2018-12-25 13:54,7.533,L
    2018-12-25 18:35,13.596,H  
  */

  while (client.connected())
  {
    const int CCH = 128;
    char rgch[CCH];

    client.readBytesUntil('\n', rgch, CCH);
    if (rgch[0] == '\r')
    {
      // A blank line indicates the end of the HTTP headers
      break;
    }
    // Get current time from HTTP header
    if (0 == strncmp(rgch, "Date:", 5))
    {
      // Skip 5 spaces to get to the time
      int cSpaces = 0;
      char *pch = rgch + 10;
      while (*pch && cSpaces < 4)
      {
        if (*pch++ == ' ')
        {
          cSpaces++;
        }
      }
      hour = atoi(pch) + CONFIG_OFFSET_FROM_UTC;
      hour = (hour + 24) % 24;

      // Now find the minute
      while (*pch++ != ':')
        ;
      minute = atoi(pch);
    }
  }
   client.readStringUntil('\n');
}
void setup(){
  Serial.begin(115200);
  if (!ConnectToWiFi())
    {
      Serial.print("notconnected");
    }
    ConnectToNoaa(client, /*out*/ hourCurrent, /*out*/ minuteCurrent);
    if(client.connected()){
      failConnect = 5;
    while (client.connected())
    {
      
      long year = client.parseInt();
      client.read(); // '-'
      long month = client.parseInt();
      client.read(); // '-'
      long day = client.parseInt();
      hour = client.parseInt();
      minute = client.parseInt();
      long level = client.parseInt();
      client.readBytesUntil(',', rgch,CCH);
      fLow = ('L' == client.read());
      client.readBytesUntil('\n', rgch, CCH);
      
      Serial.print("year");
      Serial.println(year);
      Serial.print("month");
      Serial.println(month);
     
      Serial.print("day");
      Serial.println(day);
      Serial.print("level");
      Serial.println(level);
      Serial.print("hour");
      Serial.println(hour);
      Serial.print("current hour:  ");
      Serial.println(hourCurrent);
      Serial.print("minute:");
      Serial.println(minuteCurrent);
      
     

    }
    
  client.stop();  
  Serial.print("*bool");
  Serial.println(fLow);
  //Serial.print("*tide hour");
  Serial.print("minute");
  Serial.println(minuteCurrent);
  Serial.println(hour);
  Serial.print("*currentHour");
  Serial.print(hourCurrent);
  Serial.print("*TideMinute");
  Serial.println(minute); 
  
  timerLight = hourCurrent;
  if(hourCurrent < hour) hourCurrent = hourCurrent + 24;
  hour = (hour * 60) + minute;
  hourCurrent = (hourCurrent * 60) + minuteCurrent;
  int timeToGo = hourCurrent - hour;
  if(fLow) lightLevel = map(timeToGo, 0,360, 1,60);
  else lightLevel = map(timeToGo, 0,360, 60,1);
  lightLevel = constrain(lightLevel,  1, 60);
  Serial.print("lightLevel");
  Serial.println(lightLevel);
  if((timerLight < 8) || (timerLight > 20)) timeIndex = 2;
  else if(timerLight < 11)  timeIndex = 1;
  else if(timerLight > 14) timeIndex = 4;
  else timeIndex = 3;
  Serial.print("timeIndex");
  Serial.println(timeIndex); 
}
}
void loop() {

}

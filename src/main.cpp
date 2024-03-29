/**
 * Manage summer/winter hour[OK]
 * Reset factory default [OK]
 *
 * Possibility to change the number of point on chart[TODO/2]
 * Possibility to change network parameter.[TODO]
 *
 * Show a time cursor on the chart[OK]
 *
 * Rewrite and optimize code[TODO/4]
 *
 * Do a 2.0 version when we use C++ class[TODO/5]
 *
 * Find a solution to have terminale on the Aqualight box (LCD, TFT, OLED, 7 segment)[TODO/Maybe] 1 btn to factory default,
 * 1 terminal to show error msg
 *
 * With this project we can manage led ramp of aquarium. We can use custom or generic ramp led. Possibility to tune value of light on a web page.
 * On first burn a wifi access point is created and you can connect on it. When you are connected you have to enter the SSID, PASWD of your wifi network and a static ip.
 * Finally you can access st your Aqualight box to manage led level by second.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncUDP.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Timezone.h>

#define NTPSRV "ch.pool.ntp.org"

const int lights[] = {4, 12}; // pin

const int nbrChartPoint = 6;

String ssid;
String password;

bool isAP = true;
bool connect = true;
int nWlan;

bool forceLightLevel = false;
int lightLevel = 0;

const char *locSsid = "aqualight";
const char *locPassword = "fish1234";

static IPAddress staticIP; //(192, 168, 1, 6)
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

String dataStrTab[3]; //"datas" file buffer
bool co = false;

IPAddress timeServerIP;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
AsyncUDP udp;
unsigned long epocheTime = 0;

int powerTimes[nbrChartPoint][2 /*ledID*/][2 /*h in sec; percent of light*/]; // TODO make this dynamic like a List object

unsigned int currentTime = 0;
unsigned long currentTime0 = -300000; // sendNTPpacket() at start
time_t localNow;

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone CE(CEST, CET);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
StaticJsonDocument<450> staticJson;

// Server NTP:
int getHinSec(int hour, int minute, int second) { return hour * 3600 + minute * 60 + second; }
int getHours(long epochTime) { return ((epochTime % 86400L) / 3600); }
int getMinutes(long epochTime) { return ((epochTime % 3600) / 60); }
int getSeconds(long epochTime) { return (epochTime % 60); }

unsigned long getEpocheTime(uint8_t *data)
{
  // Timestamp start at byte 40 and the length is 4 bytes
  unsigned long highWord = word(data[40], data[41]); // word is 2 Bytes
  unsigned long lowWord = word(data[42], data[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord; // Convetion of two words into a single long
  // Serial.print("Seconds since Jan 1 1900 = ");
  // Serial.println(secsSince1900);

  // now convert NTP time into UNIX timestamp:
  // Serial.print("Unix time = ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL - 7200; // Summer
  // subtract seventy years:
  unsigned long epoch = secsSince1900 - seventyYears;
  // print Unix time:
  // Serial.println("Unix timestamp: " + String(epoch));
  return epoch;
}

void (*resetFunc)(void) = 0;

void sendNTPpacket()
{
  // Serial.println("sending NTP packet...");
  //  set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // Serial.print("PacketBuffer : ");
  for (int i = 0; i < *(&packetBuffer + 1) - packetBuffer; ++i)
  {
    // Serial.print(packetBuffer[i]);
  }
  // Serial.print("\n");
  udp.write(packetBuffer, NTP_PACKET_SIZE);
}

void power(int currentSecond, int ledID)
{ // int powerTimes[nbrChartPoint0/*Number of hour key point*/][2/*ledID*/][2/*h in sec; percent of light*/];
  /*todo static point1 && point0 = point1*/
  static int ledIntensity;
  int previousLedIntensity = ledIntensity;
  int point0[2] = {powerTimes[nbrChartPoint - 1][ledID][0], powerTimes[nbrChartPoint - 1][ledID][1]}; // previous point (on chart), the LAST on the chart by default
  int point1[2] = {powerTimes[0][ledID][0], powerTimes[0][ledID][1]};                                 // next point (on chart), the FIRST on the chart by default

  int littleHInSec = -1;

  // find point0 dynamiqualy
  for (int i = 0; i < nbrChartPoint; ++i) // check every key point on chart
  {
    if (powerTimes[i][ledID][0] > -1)
    {
      if (powerTimes[i][ledID][0] <= currentSecond && powerTimes[i][ledID][0] >= littleHInSec)
      { // test all hour key of [ledID] h in sec | find previous key point
        point0[0] = powerTimes[i][ledID][0];
        point0[1] = powerTimes[i][ledID][1];

        if (i + 1 < nbrChartPoint)
        {
          point1[0] = powerTimes[i + 1][ledID][0];
          point1[1] = powerTimes[i + 1][ledID][1];
        }
        else
        {
          point1[0] = powerTimes[0][ledID][0];
          point1[1] = powerTimes[0][ledID][1];
        }

        littleHInSec = point0[0];
      }
    }
  }

  // find point 1 on chart
  /*for (int i = 0; i < nbrChartPoint; ++i) //check every key point on chart
  {
    if (powerTimes[i][ledID][0] > -1)
    {
      if (point0[0] == powerTimes[nbrChartPoint - 1][ledID][0] && point0[1] == powerTimes[nbrChartPoint - 1][ledID][1])
      {
        point1[0] = powerTimes[0][ledID][0];
        point1[1] = powerTimes[0][ledID][1];
        hugeHInSec = powerTimes[0][ledID][0];
      }
      else if (powerTimes[i][ledID][0] >= currentSecond && powerTimes[i][ledID][0] <= hugeHInSec)
      { //test all hour key of [ledID] h in sec | find high point
        point1[0] = powerTimes[i][ledID][0];
        point1[1] = powerTimes[i][ledID][1];
        hugeHInSec = powerTimes[i][ledID][0];
      }
    }
  }*/

  if ((point1[0] - point0[0]) != 0) // To be shure that we don't have division by 0
  {
    ledIntensity = (int)round(point0[1] + (currentSecond - point0[0]) * (point1[1] - point0[1]) / (point1[0] - point0[0])); // max 100
  }
  else
  {
    // ledIntensity = 0;
    ledIntensity = previousLedIntensity;
  }

  // Serial.printf("Led%d-> Point0 = [%d, %d]\t Point1 = [%d,%d] ==> %d\n", ledID, point0[0], point0[1], point1[0], point1[1], ledIntensity);

  if (ledIntensity <= 0)
  {
    digitalWrite(lights[ledID], LOW);
  }
  else if (ledIntensity >= 100)
  {
    digitalWrite(lights[ledID], HIGH);
  }
  else
  {
    ledIntensity = map(ledIntensity, 0, 100, 0, 255);
    analogWrite(lights[ledID], ledIntensity);
  }
}

void connectNTP()
{
  WiFi.hostByName(NTPSRV, timeServerIP);

  if (udp.connect(timeServerIP, 123))
  {
    // Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet)
                 {
                   epocheTime = getEpocheTime(packet.data());
                   CE.toLocal(epocheTime); });
  }
  else
  {
    // Serial.println("AsyncUDP failed to begin!");
  }
}

void infos_update()
{
  if (epocheTime > 1500000000)
  {
    power(getHinSec(getHours(epocheTime), getMinutes(epocheTime), getSeconds(epocheTime)), 0);
    power(getHinSec(getHours(epocheTime), getMinutes(epocheTime), getSeconds(epocheTime)), 1);
    epocheTime++;
    currentTime = millis();
  }
  else
  {
    connectNTP();
    currentTime0 = -300000;
  }
}

void openJson(String json, int lightIndex)
{ // receive and interpret Json from client
  // Serial.println("openJson..." + json);
  DeserializationError error = deserializeJson(staticJson, json);
  if (error)
  {
    // Serial.println("openJson Error!");
    return;
  }

  for (int i = 0; i < nbrChartPoint; ++i)
  {
    for (int j = 0; j < 2; ++j)
    { // 0 -> hinsec, 1 -> percent | 0 hinsec 1 percentoflight
      String sj = staticJson[i][j];
      powerTimes[i][lightIndex][j] = sj.toInt();
    }
  }
}

String getJson(int lightIndex)
{ // Send datas as Json to the client
  StaticJsonDocument<450> jsonObj;
  jsonObj["id"] = lightIndex;
  // Serial.println("getJson method :");
  for (int i = 0; i < nbrChartPoint; ++i)
  {
    for (int j = 0; j < 2; ++j)
    {
      jsonObj["data" + String(j)][i] = powerTimes[i][lightIndex][j];
      jsonObj["data" + String(j)][i] = powerTimes[i][lightIndex][j]; //{"id":0,"data0":[powerTimes[0][0][0],powerTimes[1][0][0],0,0,0,0],"data1":[0,14399,28798,43197,57596,71995]}
      ////Serial.print("data" + String(j) + " --> ");                     //"data0":[powerTimes[0][0][0], powerTimes[1][0][0], powerTimes[2][0][0], powerTimes[3][0][0],...]
      ////Serial.print(powerTimes[i][lightIndex][j]);                     //"data1":[powerTimes[0][0][1], powerTimes[1][0][1], powerTimes[2][0][1], powerTimes[3][0][1],...]
    }
    // Serial.print("\n");
  }

  String jsonString = "";

  serializeJson(jsonObj, jsonString);
  return jsonString;
}

void netStart()
{
  // Serial.println("-------------------------------- netStart() --------------------------------");
  // Serial.print("SSID: ");
  // Serial.println(ssid);
  // Serial.print("Pass: ");
  // Serial.println(password);
  // Serial.print("IP: ");
  // Serial.println(staticIP);

  WiFi.mode(WIFI_STA); // get available WLAN
  WiFi.disconnect();
  delay(100);
  nWlan = WiFi.scanNetworks();

  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(gateway, gateway, subnet);
  WiFi.softAP(locSsid, locPassword);
}

void netStart(String ssid, String pass, IPAddress ip)
{
  // Serial.println("-------------------------------- netStart(String ssid, String pass, String ip) --------------------------------");

  // Serial.print("SSID: ");
  // Serial.println(ssid);
  // Serial.print("Pass: ");
  // Serial.println(pass);
  // Serial.print("IP: ");
  // Serial.println(ip);
  // Serial.println("Wifi client:");

  // Serial.println("deco");
  WiFi.config(ip, dns, gateway, subnet); // set a static IP
  // Serial.println("Wifi Begin:");
  WiFi.begin(ssid, pass);

  long waitingDif = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    // Serial.print(".");
    delay(300);
    if (millis() - waitingDif >= 10000)
    {
      break;
    }
  }
  // Serial.print("\n");
  // Serial.println(WiFi.status());

  if (WiFi.status() == WL_CONNECTED)
  {
    // Serial.print("Adresse IP: ");
    // Serial.println(WiFi.localIP());
    NBNS.begin("ESP Aqualight");

    //(assync) NTP client
    // connectNTP();

    isAP = false;
    // Serial.println(WiFi.localIP());
    WiFi.softAPdisconnect(true);
  }
  else
  {
    WiFi.disconnect();
  }
}

void writeDatasIntoFile()
{

  File datasDFile = LittleFS.open("/datas.d", "w");

  // Serial.println("/datas.d is writting:");
  for (int i = 0; i < 3; ++i)
  {
    datasDFile.println(dataStrTab[i] + '\0');
  }
  datasDFile.close();

  File datas = LittleFS.open("/datas.d", "r");
  //Serial.println("/datas.d content-------yayayayayayayayayayayayaya:");

  /*while (datas.available())
  {
    Serial.print((char)datas.read());
  }*/
  // Serial.println("-------");

  datas.close();
}

void resetFile()
{
  LittleFS.remove("/datas.d");
  File datasD = LittleFS.open("/datas.d", "w");
  datasD.close();
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  { // On connection server send data as Json to the client
    if (isAP)
    {
      // client->text("ssid{wewrer, ufughufhguf, Braillard, sdfsdfds}");
      String ssidList[nWlan];
      String ssidLStr = "";
      // int sizeOfssidList = *(&ssidList + 1)/*Address after elements of the array*/ - ssidList /*Addr of the first element from array*/;
      for (int i = 0; i < nWlan; ++i)
      {
        ssidList[i] = WiFi.SSID(i);
        ssidLStr += WiFi.SSID(i) + ";";
      }
      // Serial.println(ssidLStr);
      client->text(ssidLStr);
    }
    else
    {
      client->text(getJson(0));
      client->text(getJson(1));
      client->text("lightLevel:" + String(lightLevel) + ",forceLight:" + String(forceLightLevel));
      //Serial.println("Texte in file:");
      //Serial.println(getJson(0));
      //Serial.println(getJson(1));
    }
  }
  else if (type == WS_EVT_DATA)
  { //---------------------------------------------------------------------Received data format
    // String dataStr = String((char *)&data[0]);  //add random char at end
    char dataBufer[len];
    for (uint8_t i = 0; i < len; ++i)
    {
      dataBufer[i] = data[i];
    }
    dataBufer[len] = '\0';

    String dataStr = dataBufer;

    // Serial.println("WS_DATA = " + dataStr);

    /*if (dataStr.substring(0, 1).equals("0"))
    { //----------------------------------------------------------change light 0 configuration
      int indexChar = 0;
      for (unsigned int i = 0; i < dataStr.length(); ++i)
      {
        if (dataStr.charAt(i) == ']')
        {
          indexChar = i;
        }
      }
      dataStr = dataStr.substring(0, indexChar + 1);

      dataStrTab[1] = dataStr;
      openJson(dataStrTab[1].substring(1), dataStrTab[1].substring(0, 1).toInt());

      writeDatasIntoFile();
    }
    else if (dataStr.substring(0, 1).equals("1"))
    { //----------------------------------------------------change light 1 configuration
      int indexChar = 0;
      for (unsigned int i = 0; i < dataStr.length(); ++i)
      {
        if (dataStr.charAt(i) == ']')
        {
          indexChar = i;
        }
      }
      dataStr = dataStr.substring(0, indexChar + 1);
      dataStrTab[2] = dataStr;
      openJson(dataStrTab[2].substring(1), dataStrTab[2].substring(0, 1).toInt());

      writeDatasIntoFile();
    }*/
    //Serial.println(dataStr);

    if (dataStr.substring(0, 1).equals("0") || dataStr.substring(0, 1).equals("1")) // change light 0/1 configuration
    {
      //Serial.println("Not her please");
      uint8_t ledId = dataStr.substring(0, 1).toInt();
      //Serial.printf("led id = %d", ledId);

      int indexChar = 0;
      for (unsigned int i = 0; i < dataStr.length(); ++i)
      {
        if (dataStr.charAt(i) == ']')
        {
          indexChar = i;
        }
      }
      dataStr = dataStr.substring(0, indexChar + 1);

      dataStrTab[ledId + 1] = dataStr;
      openJson(dataStrTab[ledId + 1].substring(1), dataStrTab[ledId + 1].substring(0, 1).toInt());
      writeDatasIntoFile();
    }
    else if (dataStr.substring(0, 4).equals("true"))
    { //------------------------------------toggle forced light
      // Serial.println("true  ---->> " + dataStr);
      forceLightLevel = true;
    }
    else if (dataStr.substring(0, 5).equals("false"))
    {
      // Serial.println("false ---->> " + dataStr);
      forceLightLevel = false;
    }
    else if (dataStr.substring(0, dataStr.indexOf(':')).equals("lightLevel"))
    {
      // Serial.println("lightLevel ---->> " + dataStr);
      lightLevel = dataStr.substring(11, 14).toInt();
      // Serial.println(lightLevel);
    }
    else if (dataStr.equals("reset"))
    { //--------Reset to factory default
      resetFile();
      delay(300);
      resetFunc();
    }
    else if (dataStr.equals("ssidList"))
    { //--------Rewrite WLAn Settings
      String ssidLStr = "";

      // int sizeOfssidList = *(&ssidList + 1)/*Address after elements of the array*/ - ssidList /*Addr of the first element from array*/;
      for (int i = 0; i < nWlan; ++i)
      {
        ssidLStr += WiFi.SSID(i) + ";";
        // Serial.println(WiFi.SSID(i));
      }
      // Serial.println(ssidLStr);
      client->text(ssidLStr);
      //Serial.println("SSIDLIST SENT");
    }
    else if(!dataStr.equals("undefined"))
    { //-----------------------------------------Network configuration
      int indexChar = 0;
      for (unsigned int i = 0; i < dataStr.length(); ++i)
      {
        if (isDigit(dataStr.charAt(i)))
        {
          indexChar = i;
        }
      }
      dataStr = dataStr.substring(0, indexChar + 1);
      dataStrTab[0] = dataStr; // ssid:pass:ip
      String netConf[3];       //{ssid,pass,ip}

      int index[2] = {-1, 0};
      int j = 0;
      for (unsigned int i = 0; i < dataStrTab[0].length(); ++i)
      {
        if (dataStrTab[0].substring(i, i + 1) == ":")
        {
          if (index[1] == 0)
          {
            index[1] = i;
          }
          else
          {
            index[0] = index[1];
            index[1] = i;
          }
          netConf[j] = dataStrTab[0].substring(index[0] + 1, index[1]);
          // Serial.println("netConf[" + String(j) + "] = " + String(netConf[j]));
          ++j;
        }
      }
      writeDatasIntoFile();
      netConf[j] = dataStrTab[0].substring(index[1] + 1);

      int ip[4] = {0, 0, 0, 0};

      index[0] = -1;
      index[1] = 0;
      j = 0;
      for (unsigned int i = 0; i < netConf[2].length(); ++i)
      {
        if (netConf[2].substring(i, i + 1) == ".")
        {
          if (index[1] == 0)
          {
            index[1] = i;
          }
          else
          {
            index[0] = index[1];
            index[1] = i;
          }
          ip[j] = netConf[2].substring(index[0] + 1, index[1]).toInt();
          // Serial.println("ip[" + String(j) + "] = " + String(ip[j]));
          ++j;
        }
      }

      ip[j] = netConf[2].substring(index[1] + 1).toInt();

      IPAddress ipAddr(ip[0], ip[1], ip[2], ip[3]);
      ssid = netConf[0];
      password = netConf[1];
      staticIP = IPAddress(ip[0], ip[1], ip[2], ip[3]);
      delay(50);
      Serial.println("Putin de pute!");
      //netStart(ssid, password, staticIP);

      co = true;
    }
  }
}

void loadDatasFromFile(String fileName0)
{ // gérer si datas.d est incomplet
  String fileName;
  if (fileName0.substring(0, 1).equals("/"))
  {
    fileName = fileName0;
  }
  else
  {
    fileName = "/" + fileName0;
  }

  File datas = LittleFS.open(fileName, "r");
  String fileContent = "";

  while (datas.available())
  {
    char fc = (char)datas.read();
    if (fc == '\n' || isGraph(fc))
    {
      fileContent += fc;
    }
  }
  datas.close();
  // Serial.println("fileContent : \n" + fileContent);

  // String lines[3];
  int index[2] = {-1, 0};
  int j = 0;

  for (unsigned int i = 0; i < fileContent.length(); ++i)
  {
    ////Serial.println("pos = " + String(i));
    if (fileContent.substring(i, i + 1).equals("\n"))
    {
      if (index[1] != 0)
      {
        index[0] = index[1];
      }
      index[1] = i;
      dataStrTab[j] = fileContent.substring(index[0] + 1, index[1]);
      // dataStrTab[j] = lines[j];
      ++j;
    }
  }

  // lines[j] = fileContent.substring(index[1]+1);

  // Serial.println("dataStrTab[3] : ");
  for (int i = 0; i < 3; ++i)
  {
    //_Serial.println("lines" + lines[i]);
    // Serial.println("dataStrTab " + String(i) + " : " + dataStrTab[i]);
  }

  for (int i = 1; i < 3; ++i)
  {
    if (dataStrTab[i] != "")
    {
      openJson(dataStrTab[i].substring(1), dataStrTab[i].substring(0, 1).toInt());
    }
  }

  if (dataStrTab[0] != "")
  {
    String netData[3]; /////////////////////////////////////////////////////////////
    int ipInt[4];

    index[0] = -1;
    index[1] = 0;
    j = 0;

    for (unsigned int i = 0; i < dataStrTab[0].length(); ++i)
    {
      if (dataStrTab[0].substring(i, i + 1).equals(":"))
      {
        if (index[1] != 0)
        {
          index[0] = index[1];
        }
        index[1] = i;
        netData[j] = dataStrTab[0].substring(index[0] + 1, index[1]);
        ++j;
      }
    }
    netData[j] = dataStrTab[0].substring(index[1] + 1);

    // IP
    index[0] = -1;
    index[1] = 0;
    j = 0;

    for (unsigned int i = 0; i < netData[2].length(); ++i)
    {
      if (netData[2].substring(i, i + 1).equals("."))
      { ///////////////test changé icic
        if (index[1] != 0)
        {
          index[0] = index[1];
        }
        index[1] = i;
        ipInt[j] = netData[2].substring(index[0] + 1, index[1]).toInt();
        ++j;
      }
    }
    ipInt[j] = netData[2].substring(index[1] + 1).toInt();

    /*//Serial.println("netData[3]:");
    for(int i = 0; i < 3; ++i){
      //Serial.println(netData[i]);
    }*/

    // Serial.println("ipInt[4]:");
    for (int i = 0; i < 4; ++i)
    {
      // Serial.println(ipInt[i]);
    }

    ssid = netData[0];
    password = netData[1];
    staticIP = IPAddress(ipInt[0], ipInt[1], ipInt[2], ipInt[3]);

    netStart(ssid, password, staticIP);
  }
}

void setup()
{
  //Serial.begin(9600);
  // Serial.println("//Serial start:");

  netStart();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  for (int i = 0; i < 2; ++i)
  { // lights OFF
    pinMode(lights[i], OUTPUT);
    digitalWrite(lights[i], 0);
  }

  analogWriteRange(100);

  int time = 0;

  for (int i = 0; i < nbrChartPoint; ++i)
  { // powerTimes[nbrChartPoint/*Number of hour key point*/][2/*ledID*/][2/*h in sec; percent of light*/];
    for (int j = 0; j < 2; ++j)
    {
      powerTimes[i][0][1] = 0;
      powerTimes[i][j][0] = time;
      powerTimes[i][1][1] = 10;
      powerTimes[i][j][0] = time;
    }
    time += 14400;
  }

  if (!LittleFS.begin())
  {
    // Serial.println("Erreur LittleFS...");
    return;
  }

  loadDatasFromFile("/datas.d");

  String filesName[] = {"/datas.d", "/index.html", "/style.css", "/script.js", "/wifi_config_script.js", "/wifi_config.html"};

  File file;

  int sizeOfFilesName = *(&filesName + 1) /*Address after elements of the array*/ - filesName /*Addr of the first element from array*/; // REF

  for (int i = 0; i < sizeOfFilesName; ++i)
  {
    file = LittleFS.open(filesName[i], "r");
    if (!file)
    {
      // Serial.print("Failed to open file \"");
      // Serial.print(filesName[i]);
      // Serial.println("\" to read it. :(");
      isAP = true;
      break;
    }
    // Serial.println(file.name());
    file.close();
  }

  /*////Serial.println("File Content:");

  while (file.available()) {

    //Serial.write(file.read());
  }*/

  //-------------- SERVER ------------//
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (isAP)
              {
                request->send(LittleFS, "/wifi_config.html", "text/html");
              }
              else
              {
                //Serial.println("send /index.html");
                request->send(LittleFS, "/index.html", "text/html");
                //request->send(LittleFS, "text/html", "/index.html");
              } });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/style.css", "text/css"); });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/script.js", "text/javascript"); });

  server.on("/wifi_config_script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/wifi_config_script.js", "text/javascript"); });

  server.begin();

  // setenv("TZ", MY_TZ, 1);
}

void loop()
{
  if (millis() - currentTime >= 1000)
  {
    if (!isAP)
    {
      if (forceLightLevel)
      {
        analogWrite(lights[0], map(lightLevel, 0, 100, 0, 255));
        analogWrite(lights[1], map(lightLevel, 0, 100, 0, 255));
      }
      else
      {
        infos_update();
        // time(&localNow);                   //get UTC time

        // Serial.print("epocheTime = localNow = ");
        // Serial.println(epocheTime);
        // Serial.print(getHours(epocheTime));
        // Serial.print(":");
        // Serial.print(getMinutes(epocheTime));
        // Serial.print(":");
        // Serial.println(getSeconds(epocheTime));

        currentTime = millis();
      }
    }
  }

  if (millis() - currentTime0 >= 60000)
  { // 1 min
    if (!isAP)
    {
      sendNTPpacket(); // send NTP packet to update time if is it false.
      currentTime0 = millis();
    }
  }

  if (co)
  {
    netStart(ssid, password, staticIP);
    co = false;
  }
}
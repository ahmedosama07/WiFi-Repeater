
// NAPT example released to public domain

#if LWIP_FEATURES && !LWIP_IPV6

#define HAVE_NETDUMP 0

String ssid = "";
String pass = "";

unsigned long timestart = 0;
unsigned long timeout=0;

int n = 0;
int trial = 0;

#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>

#define NAPT 1000
#define NAPT_PORT 10

#if HAVE_NETDUMP

#include <NetDump.h>

void dump(int netif_idx, const char* data, size_t len, int out, int success) {
  (void)success;
  Serial.print(out ? F("out ") : F(" in "));
  Serial.printf("%d ", netif_idx);

  // optional filter example: if (netDump_is_ARP(data))
  {
    netDump(Serial, data, len);
    // netDumpHex(Serial, data, len);
  }
}
#endif

void wifiScan();
void wifiInit();

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

#if HAVE_NETDUMP
  phy_capture = dump;
#endif

  

  // first, connect to STA so we can get a proper local DNS server
  wifiScan();
  wifiInit();
  Serial.printf("\nSTA: %s (dns: %s / %s)\n", WiFi.localIP().toString().c_str(), WiFi.dnsIP(0).toString().c_str(), WiFi.dnsIP(1).toString().c_str());

  // By default, DNS option will point to the interface IP
  // Instead, point it to the real DNS server.
  // Notice that:
  // - DhcpServer class only supports IPv4
  // - Only a single IP can be set
  auto& server = WiFi.softAPDhcpServer();
  server.setDns(WiFi.dnsIP(0));

  WiFi.softAPConfig(  // enable AP, with android-compatible google domain
    IPAddress(172, 217, 28, 254), IPAddress(172, 217, 28, 254), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid + " repeater", pass);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) { Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", ssid + " repeater", pass); }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) { Serial.printf("NAPT initialization failed\n"); }
}

#else

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT not supported in this configuration\n");
}

#endif

void loop() {}


void wifiScan()
{
    n = WiFi.scanNetworks();
  int x = WiFi.scanComplete();
  Serial.println(x);

  Serial.println("Scan started");
  delay(500);

  if(n == 0)
  {
    Serial.println("Not networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");

    for(int i = 0; i < n; i++)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      if((WiFi.encryptionType(i) == ENC_TYPE_NONE))
      {
        Serial.println("*");
      }
      delay(10);
      Serial.println("");
    }
  }

  Serial.println("Enter the no. of the network you want to connect");
  while (Serial.available() == 0){}
  int no_ssid = Serial.parseInt();
  ssid = WiFi.SSID(no_ssid - 1);  

  if((WiFi.encryptionType(no_ssid - 1) != ENC_TYPE_NONE))
  {
    Serial.println("Please enter the password of the network you chose");
    while (Serial.available() == 0){}
    pass = Serial.readStringUntil('\n');
  }
  else
  {
    pass = "";
  }
}

void wifiInit()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wifi...");
  timeout = millis();
  while((WiFi.status() != WL_CONNECTED) && (millis() - timeout < 8000))
  {
    Serial.print(".");
    delay(1000);
  }
  if(WiFi.status() != WL_CONNECTED && WiFi.status() != WL_NO_SSID_AVAIL)
  {
    Serial.println("Password is not correct");
  }
  else if(WiFi.status() != WL_CONNECTED && WiFi.status() == WL_NO_SSID_AVAIL)
  {
    Serial.println("Wifi network is not avaliable");
  }
  else
  {
    Serial.println("");
    Serial.println("Connected successfully");
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
    trial = 0;
  }

}

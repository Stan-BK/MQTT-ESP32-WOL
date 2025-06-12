#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WakeOnLan.h>

const char *MACAddress = "**-**-**-**-**-**";

// WiFi
const char *ssid = "***";            // Enter your WiFi name
const char *password = "***"; // Enter WiFi password
WiFiUDP UDP;
WakeOnLan WOL(UDP); // Pass WiFiUDP class

// MQTT Broker
const char *mqtt_broker = "***"; // Enter MQTT Broker server uri
const char *topic = ""; // Client Start-up topic
const char *res_topic = ""; // Host response topic
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char *root_ca = R"EOF(-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----)EOF";

void connectToWifi();

void connectToMQTT();

void callback(char *topic, byte *payload, unsigned int length);

void wakePC();

void setup()
{
  pinMode(2, OUTPUT);
  if (!Serial)
    Serial.end();
  // Set software serial baud to 115200;
  Serial.begin(115200);

  connectToWifi();

  espClient.setCACert(root_ca);
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setKeepAlive(60);
  client.setCallback(callback);

  connectToMQTT();

  digitalWrite(2, HIGH);
}

void connectToWifi()
{
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void connectToMQTT()
{
  while (!client.connected())
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the mqtt broker: %s\n", client_id.c_str(), mqtt_broker);
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state() + "\n");
      delay(2000);
    }
  }
  // publish and subscribe
  client.subscribe(topic);
  client.publish(res_topic, "Hi EMQX I'm ESP32 ^^ \n");
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  client.publish(res_topic, "Wake up computer! \n");
  wakePC();
  Serial.println("-----------------------");
}

void wakePC()
{
  WOL.sendMagicPacket(MACAddress);
}

void loop()
{
  while (!client.connected())
  {
    connectToMQTT();
  }
  client.loop();
}
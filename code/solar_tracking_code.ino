#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define volt A0
#define crnt D3
#define ldr1 D2
#define ldr2 D1
#define mot D8
#define gnd D7
#define con1 D5
#define con2 D6
int l1;
int l2;
String volt1;
String crnt1;
String buf1;
String buf2;
const char* ssid = "";
const char* password = "";
//const char* mqtt_server = "";
const char* mqtt_server = "";
//const char* mqtt_server = "";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Command from MQTT broker is : [");
  Serial.print(topic);
  int p = (char)payload[0];
  if (p == '0')
  {
    digitalWrite(mot, LOW);
    //digitalWrite(LED_BUILTIN, LOW);
    Serial.println(" Turn OFF motor " );
    client.publish("solar1", "MOTOR OFF ");
  }
  if (p == '1')
  {
    digitalWrite(mot, HIGH);
    //digitalWrite(LED_BUILTIN, HIGH);
    Serial.println(" Turn on motor" );
    client.publish("solar1", "MOTOR ON");
  }
  Serial.println();
} //end callback
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "solar";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe("solar/tst");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup() {

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(ldr1, INPUT);
  pinMode(ldr2, INPUT);
  pinMode(crnt, INPUT);
  pinMode(volt, INPUT);
  pinMode(mot, OUTPUT);
  pinMode(con1, OUTPUT);
  pinMode(con2, OUTPUT);
  digitalWrite(con1, LOW);
  digitalWrite(con2, LOW);
  pinMode(gnd, OUTPUT);
  digitalWrite(gnd, LOW);
}

void loop() {
  digitalWrite(gnd, LOW);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    float analogvalue = analogRead(A0);
    float vol = (analogvalue / 100) / 1.47;
    float crt = (vol / 50) * 1000;
    char message[9];
    char message1[11];
    char message2[4];
    char message3[4];
    l1 = digitalRead(ldr1);
    l2 = digitalRead(ldr2);
    volt1 = vol;
    volt1 += F(" V");
    crnt1 = crt;
    crnt1 += F(" mA");
    volt1.toCharArray(message, 9);
    crnt1.toCharArray(message1, 11);
    Serial.println(volt1);
    Serial.println(crnt1);
    client.publish("solar2", message);
    client.publish("solar3", message1);
    Serial.print("voltage=");
    Serial.println(vol);
    Serial.print("current=");
    Serial.println(crt);
    Serial.print("ldr1=");
    Serial.println(l1);
    Serial.print("ldr2=");
    Serial.println(l2);
    delay(50); //Delay 50 ms
    if (l1 == 0)
    {
      client.publish("solar4", "LDR1 sensed");
      digitalWrite(con1, HIGH);
      digitalWrite(con2, LOW);
      delay(200);
    }
    if (l2 == 0)
    {
      client.publish("solar4", "LDR2 sensed");
      digitalWrite(con1, LOW);
      digitalWrite(con2, HIGH);
      delay(200);
    }
    if ((l1 == 1) && (l2 == 1))
    {
      digitalWrite(con1, LOW);
      digitalWrite(con2, LOW);
      
    }
  }

}

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
//Wi-Fi Connection Data
const char* ssid     = "MasQuem";      // SSID of local network
const char* password = "chupamaria92";   // Password on network
//HTTP Request Data
const char* host = "192.168.0.101";
const char* url = "/ss/ws.php";
const int httpPort = 80;
String data = "action=checkModule&id_user=19";
//System Variables
const size_t bufferSize = JSON_OBJECT_SIZE(4) + 80;
WiFiClient client;
int lastReq;
int val;
byte relayPin[4] = {2,7,8,10};
int RELAY_P = D8;
//D2 -> RELAY1
//D7 -> RELAY2
//D8 -> RELAY3
//D10 -> RELAY

void setup() 
{ 
  //Serial.begin(115200);
  Serial.begin(9600);

  delay(10);

  pinMode(RELAY_P, OUTPUT);
  digitalWrite(RELAY_P, LOW);
  Serial.println("Relay Shield ok! \nWaiting for input.");
  
  Serial.println("Connecting to");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
  delay(1000);

  /*
  Serial.println("Configuring Relay Shield");
  for(int i = 0; i < 4; i++)  pinMode(relayPin[i],OUTPUT);
  for(int i = 0; i < 4; i++)
  {
    pinMode(relayPin[i],OUTPUT);
  }
  */
  //pinMode(relayPin[2],OUTPUT);
}

void loop() 
{
  digitalWrite(RELAY_P, LOW);
  //for(int j = 0; j < 4; j++)  digitalWrite(relayPin[j],HIGH);
  checkForRequest();
  delay(30000);  //Sends a request every 30 seconds
}

void checkForRequest ()
{
  String result;
  bool begin = false;
 if(WiFi.status()== WL_CONNECTED && client.connect(host, 80)) //Check WiFi connection status
 {   
   Serial.print("Requesting POST: \n");
   // Send request to the server:
   client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                //"Connection: close\r\n" +
                "Content-Type: application/x-www-form-urlencoded\r\n" +
                "Content-Length: " + data.length() + "\r\n" +
                "\r\n" + // This is the extra CR+LF pair to signify the start of a body
                data + "\n");
                 
   //delay(10); // Can be changed

   int waitcount = 0;
   while (!client.available() && waitcount++ < 100) 
   {
      delay(10);
   }

   // Read all the lines of the reply from server and print them to Serial
   //Serial.println("Response: \n");
   while(client.connected() && !client.available()) delay(1); //waits for data
   while (client.connected() || client.available()) 
   { //connected or data available
     char c = client.read(); //gets byte from ethernet buffer
     if (c == '{') {
        begin = true;
    }

    if (begin) result += (c);

    if (c == '}') {
        break;
    }

    delay(1);
     //Serial.println(c);
   }

   client.stop(); //stop client
   result.replace('[', ' ');
   result.replace(']', ' ');
   Serial.println("\nJSON Recebido: ");
   Serial.println(result + "\n");
  
   StaticJsonBuffer<bufferSize> json_buf;
   JsonObject& root = json_buf.parseObject(result);
   if (!root.success())
   {
     Serial.println("parseObject() failed");
   }
   delay(5000);
   int id_requisicao = root["id_rastreabilidade"];
   int id_usuario = root["id_usuario"];
   String id_modulo = root["id_modulo"];
   String descricao = root["descricao"];  
   //char mdl = (char) id_modulo;
   Serial.println("Descrição do módulo acionado: \n" + descricao + "\n" + "ID Módulo: " + id_modulo + " \n");

   relayControl(id_requisicao, id_modulo);
   /*EEPROM Save & Load Tests:*/
   //saveRequestData(id_requisicao);
   //int test = int(loadRequestData());
   //Serial.println(test);
 }
 else
 {  
    Serial.println("Error in WiFi connection");   
 }
}

/** Store Last Request ID to EEPROM */
void saveRequestData(int id_rastreabilidade) 
{
  EEPROM.begin(512);
  EEPROM.put(0, id_rastreabilidade);
  //EEPROM.put(0+sizeof(id_rastreabilidade));
  //char ok[2+1] = "OK";
  //EEPROM.put(0+sizeof(id_rastreabilidade), ok);
  EEPROM.commit();
  EEPROM.end();
}
/** Retrieve Last Request ID to EEPROM */
int loadRequestData ()
{
  int customVar;
  EEPROM.begin(512);
  EEPROM.get(0, customVar);
  //EEPROM.get(0+sizeof(id_rastreabilidade));
  //char ok[2+1];
  //EEPROM.get(0+sizeof(id_rastreabilidade), ok);
  EEPROM.end();
  //if (String(ok) != String("OK")) {
  //  id_rastreabilidade[0] = 0;
  //}
  return customVar;
}


void relayControl(int id_req,String id_mod)
{
  //if(id_mod != '-1')
    //{
    char copy[50];
    id_mod.toCharArray(copy, 50);
      //int swt = (int) id_mod;
      switch(copy[0])
      {
   
      case '1':                         
        Serial.println("Relay1\n");
        val=digitalRead(relayPin[0]);
        val=!val;
        digitalWrite(relayPin[0],val);
        break;
      case '2':
        Serial.println("Relay2\n");
        val=digitalRead(RELAY_P);
        val=!val;
        digitalWrite(RELAY_P,HIGH);
        delay(500);
        digitalWrite(RELAY_P,LOW);
        break;
      case '3':
        Serial.println("Relay3\n");
        val=digitalRead(relayPin[2]);
        val=!val;
        digitalWrite(relayPin[2],val);
        break;
      case '4':
        Serial.println("Relay4\n");
        val=digitalRead(relayPin[3]);
        val=!val;
        digitalWrite(relayPin[3],val);
        break;    
        
      default:
        //if(input != '\r' && input != '\n')
         Serial.println("invalid entry");
        break;
      }
    //}
}

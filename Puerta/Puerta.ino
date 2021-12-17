#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

WiFiClient wifiClient;

#include <SPI.h>
#include <RFID.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//-------------------- RED -------------------
// Declaracion e inicializacion de variables utilizadas para la programación
// Datos para conectarse a una red wifi ya creada.
const char WiFiSSID[] = "RSE_cantv"; //SSID de la red Wifi//Conexión con mi red wifi desde la cual se puede crear un servidor para monitorear valores
const char WiFiPSK[] = "3n1@c3m7";  //Contraseña WPA, WPA2 o WEP
const char WiFiAPPSK[] = "123";

//configuracion soft-AP
const char ssid[]="RSE_P";            // nombre de la red wifi creada
const char password[]="puertaopen";      // contraseña de la red wifi creada


HTTPClient http;

String GetUrl;
String response;

// Declaracion del objeto que actua como servidor y configura 
// el puerto 80 que es la que respondera a solicitudes HTTP

/*
//Datos para una IP estática
IPAddress ip(192,168,0,10);     
IPAddress gateway(192,168,0,1);   
IPAddress subnet(255,255,255,0); 
*/
WiFiServer server(80);
//---------------------------------------------
#define SDA_DIO 15
#define RESET_DIO 5


RFID mfrc522(SDA_DIO, RESET_DIO);

#define USERS 2
byte validKey1[USERS][5] = {{0x93,0xA6,0x83,0x24,0x92},{0x4D,0x19,0xC8,0x49,0xD5}};
                            
char* names[USERS]={"Beto","Pirela"}; 

int LedRojo=0;
int LedVerde=4;
int Rele=16;

//-------------------------------------------
void setup()
{ 
  //Wire.begin();
  Serial.begin(9600);
  /* Enable the SPI interface */
  SPI.begin(); 
  /* Initialise the RFID reader */
  mfrc522.init();

  pinMode(LedRojo, OUTPUT);  //Inicializacion de pines
  pinMode(LedVerde, OUTPUT);
  pinMode(Rele, OUTPUT);
  digitalWrite(LedRojo, LOW);
  digitalWrite(LedVerde, LOW);
  digitalWrite(Rele, LOW);

  //----------------------RED-----------------------
  connectWiFi();
  server.begin();
  GetUrl = "http://quan.suning.com/getSysTime.do";
  http.setTimeout(5000);
  http.begin(wifiClient,GetUrl);
//--------------------------------------------
}

void loop()
{

  //-----------------------RED---------------------
  WiFiClient client = server.available();
  /*if (!client) {
    Serial.println("no conecta");
    return;
  }*/
  

  if (mfrc522.isCard()) /* Card Exist? */
  {
    mfrc522.readCardSerial();   /* Get serial number Card*/
    //Serial.println("CONTROL DE ACCESO");

    //imprimir codigo de tarjeta
    for(int i=0;i<5;i++)
    {
    //Serial.println(RC522.serNum[i],DEC);
      Serial.println(mfrc522.serNum[i],HEX); //Card in Hexadecimal
    }
    //----------------------------------------------
     if(checkAuthorization(mfrc522.serNum))
    { 
      //------------------Hora---------------------
      int httpCode = http.GET();
      if (httpCode > 0){
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {

        // Leer el contenido de la respuesta

        response = http.getString();

        Serial.println(response);

      }

      } else {

        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());

      }

      http.end();

      delay(50);
  //-------------------------------------------
      Serial.println("Acceso: Autorizado");
      digitalWrite(LedRojo, LOW);
      digitalWrite(LedVerde, HIGH);
      digitalWrite(Rele, HIGH);
      delay(1000);
      digitalWrite(LedRojo, LOW);
      digitalWrite(LedVerde, LOW);
      digitalWrite(Rele, LOW);
      
    }
    else
    {
      Serial.println("Acceso: Denegado");
      digitalWrite(LedRojo, HIGH);
      digitalWrite(LedVerde, LOW);
      digitalWrite(Rele, LOW);
      delay(1000);
      digitalWrite(LedRojo, LOW);
      digitalWrite(LedVerde, LOW);
      digitalWrite(Rele, LOW);
          
    }
   
    Serial.println();
    Serial.println();
  }

  client.println("HTTP/1.1 200 OK"); // La respuesta empieza con una linea de estado  
  client.println("Content-Type: text/html"); //Empieza el cuerpo de la respuesta indicando que el contenido será un documento html
  client.println(""); // Ponemos un espacio
  client.println("<!DOCTYPE HTML>"); //Indicamos el inicio del Documento HTML
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"); //Para que se adapte en móviles
  client.println("<title>Acceso principal a las oficinas de RSE</title>");
  client.println("</head>");
  client.println("<body>");
  client.println("<br><br>");
  
  client.println("<h1 style=\"text-align: center;\">Acceso principal a las oficinas de RSE</h1>");
  
  client.println("<p style=\"text-align: center;\">");
  client.println("Ultimo usuario en entrar: <br> <br>"+lastuser(mfrc522.serNum)+" "+response.substring(13, 23)+" "+response.substring(24, 32));

  client.println("</p>");
  client.println("</body>");
  
  client.println("</html>"); //Terminamos el HTML
 
  delay(1);
  //Serial.println("Cliente desconectado"); //Imprimimos que terminó el proceso con el cliente desconectado
  //Serial.println("");

  delay(20);
  
}


bool checkAuthorization(byte *data) //Verify Access
{
    for(int i = 0; i<USERS; i++)
    {
      if(checkData(data,validKey1[i],5))
      {
        Serial.print("Usuario: ");
        Serial.println(names[i]);

        return true;
      }
    }
   return false;
}
String lastuser(byte *data)
{
    for(int i = 0; i<USERS; i++)
    {
      if(checkData(data,validKey1[i],5))
      {
        //Serial.print("Usuario: ");
        //Serial.println(names[i]);

        return names[i];
      }
    }
   return "ninguno";
}
bool checkData(byte* arrayA, byte* arrayB, int length) //compare two Array
{
  for (int index = 0; index < length; index++)
  {
    if (arrayA[index] != arrayB[index]) return false;
  }
  return true;
}
//------------------- RED ---------------------
void connectWiFi()
{
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WiFiSSID, WiFiPSK);
  WiFi.softAP(ssid, password, 1, false, 2);

   while (WiFi.status() != WL_CONNECTED) 
  {
    delay(100); 
   Serial.print('.');
  }
  Serial.print("conecto!");
  Serial.println(WiFi.localIP());
  
}


/*
PINOUT:
RC522 MODULE    Uno/Nano     MEGA
SDA             D10          D9
SCK             D13          D52
MOSI            D11          D51
MISO            D12          D50
IRQ             N/A          N/A
GND             GND          GND
RST             A0           D8
3.3V            3.3V         3.3V
*/

#include <SPI.h>
#include <RFID.h>
//#include <Wire.h>

#define SDA_DIO 15
#define RESET_DIO 5


RFID mfrc522(SDA_DIO, RESET_DIO);

#define USERS 2
byte validKey1[USERS][5] = {{0x93,0xA6,0x83,0x24,0x92},{0x4D,0x19,0xC8,0x46,0xD5}};
                            
char* names[USERS]={"Beto","Pirela"}; 

int LedRojo=0;
int LedVerde=4;
int Rele=16;

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

}

void loop()
{

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

bool checkData(byte* arrayA, byte* arrayB, int length) //compare two Array
{
  for (int index = 0; index < length; index++)
  {
    if (arrayA[index] != arrayB[index]) return false;
  }
  return true;
}

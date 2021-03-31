#include <SoftwareSerial.h>
SoftwareSerial moduleWifi(3, 2); // TX | RX
#include <DHT.h>
#define DHTPIN 12
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);


int state = 0;
float setFan = 32;
float temp=0;
int id;

void setup()
{ 
  Serial.begin(115200);
  moduleWifi.begin(115200);
  dht.begin();
  pinMode(11, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  configModuleWifi();
  
}
 
void loop()
{
   readTemp();
   //String datos[2];
   //datos[2]= readModuleWifi();
   String cadena = readModuleWifi();
   int var = 0;
   var= readGet(cadena);
   switch (var) {
    
    case -2:
    turnOffOnError();
    break;

    case 2:
    //state = 1;
    startSystem();
    break;

    case 3:
    //state = 0;
    stopSystem();
    break;

    case 4:
    setTempeture();
    break;

    case 5:
    getTempeture(state);
    break;

    default:
    //Serial.println("--");
    break;
   }
   

}

String readModuleWifi(){
  //String datos[2];
  char cal;
  String cadena = "";
  while (moduleWifi.available() == 0) {
    //nada
    readTemp();
  }
    if (moduleWifi.available() > 0) {
      if(moduleWifi.find("+IPD,")){
        delay(100);
        cadena = "";
        id = moduleWifi.read()-48;
        delay(100);
        moduleWifi.find("/");
        cadena= moduleWifi.readString();
        delay(1000);
        Serial.println("---> " + cadena + "--- >>" + id);
        //decideTemp(cadena);        
        return cadena;
        }  
      } 
 }

  int readGet(String cadena){
    if(cadena.length() > 0){
      if (cadena.startsWith("holaMundo",0)){
        //Serial.println("1!");
        //state = true;
        return 1;
        }
      if (cadena.startsWith("start",0)){
        //Serial.println("2!");
        //state = false;
        return 2;
        }
      if (cadena.startsWith("stop",0)){
        //Serial.println("2!");
        //state = false;
        return 3;
        }
      if (cadena.startsWith("set",0)){
        //setFan=cadena.toInt().substring(2,3);
        String cade=cadena.substring(4,6);
        setFan=cade.toInt();
        Serial.println("setFan");
        Serial.println(setFan);
        return 4;
        }
      if (cadena.startsWith("get",0)){
        return 5;
        }
      if (cadena.startsWith("start",0) == false && cadena.startsWith("stop",0) == false && cadena.startsWith("set",0) == false && cadena.startsWith("get",0) == false){
        return -2;
        }
      }
      return 0;
    }

  void startSystem(){
    state = 1;
    responseClient("<h1>Sistema encendido !</h1>" );
    delay(500);
    turnOnTestigo();
    turnOnAlert();
    //responseClient("<h1>Sistema encendido:!</h1>\n <h2>Por defecto se tomara la temperatura limite de 32 grados para encender el ventilador </h2> <h3> - Si desea modificarla escriba set=TEMPERATURA_NUMERO!</h3>\n<h3>- Si desea vizualizar la temperatura actual escriba get!</h3>\n <h3>- Si desea apagar el sistema escriba stop!</h3>\n" );
    }
  void stopSystem(){
    state = 0;
    //turnOffTestigo();
    //turnOffAlert();   
    //turnOffFan(); 
    responseClient("<h1>Sistema apagado !</h1>" );
    delay(500);
    turnOffTestigo();
    turnOffAlert();
    turnOffFan();
    }
  void setTempeture(){
    String res = "<h1>La temperatura se modifico a: </h1>" ;
    res += setFan;
    responseClient(res);
    }
  void getTempeture(bool state){
    String res;
    if(state==1){
      res="<h1>La temperatura es: !</h1>";
      //readTemp();
      res+=temp;
      res+="<h2>La temperatura de activacion es: !</h2>";
      res+=setFan;
      Serial.println(temp);
      Serial.println(setFan);
      responseClient(res);      
      }else{
        res="<h1>No es posible leer la temperatura, el sistema esta apagado!</h1> <h2>Escriba el slug /run para encender el sensor</h2>";
        responseClient(res);
      }
    }
  void turnOffOnError(){
    responseClient("<h1>esta direccion no existe !</h1>" ); 
    Serial.println("a request does not exist");
    return;
  }
  
  void responseClient(String html) {
    //Serial.println(ide);
    String sendClient = "AT+CIPSEND=";
    sendClient+= id;
    sendClient+=",";
    sendClient+=html.length();
    sendClient+="\r\n";
    sendData(sendClient,2000);
    sendData(html,2000);
    
    String closeClient = "AT+CIPCLOSE="; 
    closeClient+= id;
    closeClient+="\r\n";
    sendData(closeClient,3000);
    Serial.println("fin mensaje enviado!!");
    id=0;
    return;
    }

 void configModuleWifi(){
    sendData("AT+RST\r\n",2000); 
    sendData("AT+CWMODE=3\r\n",1000);
    //sendData("AT+CWJAP=\"DELTA 2020\",\"Delta992020.\"\r\n",6000);
    sendData("AT+CIPMUX=1\r\n",2000);
    sendData("AT+CIPSERVER=1,80\r\n",3000); 
    sendData("AT+CIFSR\r\n",2000);
    Serial.println("--------- END CONFIG -----------");
    }

  void sendData(String comando, const int timeout) {
    long int time = millis();
    moduleWifi.print(comando); 
    while( (time+timeout) > millis())
    {
      while(moduleWifi.available())
      { // Leer los datos disponibles
        char c = moduleWifi.read();
        Serial.print(c);
      }
    } 
    return;
  }

  void readTemp(){
    if(state==1){
      delay(1000);
    //turnOnTestigo();
    //turnOnAlert();
    temp = dht.readTemperature();
    }else if (state==0){
      delay(1000);
      //turnOffTestigo();
      //turnOffAlert();
      //turnOffFan();
      Serial.println("No se puede leer temperatura porque el sistema esta apagado");
      }
      onsetFan();
      return;
    }

  void onsetFan(){
    if(state==1){
      Serial.println("onsetFan - Encender led?");
      if(temp>=setFan){
        Serial.println(temp);
        Serial.println(setFan);
         turnOffAlert();
         turnOnFan();
        }else{
          turnOnAlert();
          turnOffFan();
         }
    }
      return;
    }

 void turnOnTestigo(){
  int testigo = 9;
  digitalWrite(testigo, HIGH);
  Serial.println("led testigo encendido -> ok");
  return;
  //sendData("AT+CIPCLOSE\r\n",2000); 
  }
  
 void turnOffTestigo(){
  int testigo = 9;
  digitalWrite(testigo, LOW); 
  Serial.println("led testigo apagado -> ok");
  return;
  //sendData("AT+CIPCLOSE\r\n",2000); 
  }

 void turnOnAlert(){
  int alert = 11;
  digitalWrite(alert, HIGH);
  Serial.println("led alerta encendido -> ok");
  return;
  //sendData("AT+CIPCLOSE\r\n",2000); 
  }
  
 void turnOffAlert(){
  int alert = 11;
  digitalWrite(alert, LOW); 
  Serial.println("led alerta apagado -> ok");
  return;
  //sendData("AT+CIPCLOSE\r\n",2000); 
  }

 void turnOnFan(){
  int fan = 10;
  digitalWrite(fan, HIGH);
  Serial.println("ventilador encendido -> ok");
  return;
  //sendData("AT+CIPCLOSE\r\n",2000); 
  }
  
 void turnOffFan(){
  int fan = 10;
  digitalWrite(fan, LOW);
  Serial.println("ventilador apagado -> ok");
  return;
  //sendData("AT+CIPCLOSE\r\n",2000); 
  }

  

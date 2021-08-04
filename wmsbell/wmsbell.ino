// Скетч отправки mqtt сообщения о 
// - Звонке (код 0x30)
// - Heart beat/Beacon (код 0x31)
// - поддержка режима конфигурации (код 0x32)
// 
//

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>                            // Библиотека для создания Wi-Fi подключения (клиент или точка доступа)
#include <ESP8266WebServer.h>                       // Библиотека для управления устройством по HTTP (например из браузера)
#include <FS.h>                                     // Библиотека для работы с файловой системой
#include <ESP8266FtpServer.h>                       // Библиотека для работы с SPIFFS по FTP
#include <LittleFS.h>
#include "EspMQTTClient.h"
#include <ESP8266HTTPClient.h>

#include <PubSubClient.h>

const byte opModeBeacon = 0x30;
const byte opModeBell   = 0x31;
const byte opModeCfg    = 0x32;

/* Beacon & Bell operation part */
#define MSG_BUFFER_SIZE  (500)
unsigned long lastMsg = 0;
char          msg[MSG_BUFFER_SIZE];
int           value = 0;
int           pubs_number = 1;      // Uses to define number of publicstions in topic

const byte relay = 4;                               // Пин подключения сигнального контакта реле
const char *ssid = "ArtESP";                        // Название генерируемой точки доступа

// iotDevName Config param
const char* def_cfg_iotDevName  = "BELL_BUTTON_0";
const char* json_iotDevName     = "iotDevName";
String iotDevName               = def_cfg_iotDevName;

// iotBrockerIP Config param
const char* def_cfg_iotBrockerIP = "test.mosquitto.org";
const char* json_iotBrockerIP    = "iotBrockerIP";
String iotBrockerIP              = def_cfg_iotBrockerIP;   // Your Domain name with URL path or IP address with path 

// iotBrockerKey
const char* def_cfg_iotBrockerKey = "123456";
const char* json_iotBrockerKey    = "iotBrockerKey";
String iotBrockerKey              = def_cfg_iotBrockerKey;

// iotMqttPort
const int def_cfg_iotMqttPort     = 1883;  
const char* json_iotMqttPort      = "iotBrockerPort";
int      iotMqttPort              = def_cfg_iotMqttPort;

// iotBeaconTO
const int   def_cfg_iotBeaconTO  = 15;
const char* json_iotBeaconTO     = "iotBeaconTO";
unsigned int  iotBeaconTO        = def_cfg_iotBeaconTO;

// iotWiFiSsid
const char* def_cfg_iotWiFiSsid  = "ART-HIVE";
const char* json_iotWiFiSsid     = "iotWiFiSsid";
String  iotWiFiSsid              = def_cfg_iotWiFiSsid;

// iotWiFiKey
const char* def_cfg_iotWiFiKey   = "wifiteligent";
const char* json_iotWiFiKey      = "iotWiFiKey";
String iotWiFiKey                = def_cfg_iotWiFiKey;

// iotTopic
const char* def_cfg_iotTopic     = "ArtHive/Bell";
const char* json_iotTopic        = "iotTopic";
String iotTopic                  = def_cfg_iotTopic;

// iotSsl
const char* def_cfg_iotSsl       = "false";
const char* json_iotSsl          = "iotSsl";
String iotSsl                    = def_cfg_iotSsl;

WiFiClient espClient;
PubSubClient client(espClient);

ESP8266WebServer HTTP(80);                          // Определяем объект и порт сервера для работы с HTTP
FtpServer ftpSrv;                                   // Определяем объект для работы с модулем по FTP (для отладки HTML)

char buf[ 3 ];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("d[Connecting to ");
  Serial.print(iotWiFiSsid);
  Serial.print( " Key ");
  Serial.print(iotWiFiKey);
  Serial.println("]");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(iotWiFiSsid.c_str(), iotWiFiKey.c_str());

  Serial.print("d[");

  while (WiFi.status() != WL_CONNECTED) {
    delay( 500 );
    Serial.print(".");
  }

  Serial.println("]");

  randomSeed(micros());

  Serial.println("d[WiFi connected]");
  Serial.print("d[IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("]");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("d[Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("]");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("d[Attempting MQTT connection...");
    Serial.print("Host ");
    Serial.print( iotBrockerIP.c_str() );
    Serial.println("]");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("d[connected]");
    } else {
      Serial.print("d[failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

char modeBuf[ 6 ]={ 0, 0, 0, 0, 0, 0 };

void setup() 
{
  Serial.begin(9600);               // Инициализируем вывод данных на серийный порт со скоростью 9600 бод

  SPIFFS.begin();                   // Инициализируем работу с файловой системой                          

  loadConfig();                     // Load initial config

  Serial.print( "b[" );
  Serial.print( iotBeaconTO, DEC );
  Serial.println( "]" );
  
  /* wait mode code from master host */
  Serial.print( "d[" );
  int i;
  for( i=0;  i<100; i++)
  {
    while( Serial.available())
    {
      char chBuf;
      if( Serial.readBytes((byte*)&chBuf, sizeof(chBuf)))
      {
        for( int i=4; i>=0; i-- )
          modeBuf[ i+1 ]= modeBuf[ i ];

        modeBuf[ 0 ]= chBuf;

        if(( modeBuf[ 5 ]=='b' )
          &&( modeBuf[ 4 ]=='[' )
          &&( modeBuf[ 0 ]==']'))
        {
          buf[ 0 ]= modeBuf[ 3 ];
          buf[ 1 ]= modeBuf[ 2 ];
          buf[ 2 ]= modeBuf[ 1 ];
         
          Serial.print("]\nd[command from master[0x");
          Serial.print(buf[ 0 ], HEX);
          Serial.println("]]");
          Serial.print("d[battery[");
          Serial.print(buf[1]);
          Serial.print(buf[2]);
          Serial.print("]");         
          break;          
        }
      }else{
        Serial.println("\nd[readBytes[0]]");      
      }
    }
    if(( modeBuf[ 5 ]=='b' )
      &&( modeBuf[ 4 ]=='[' )
      &&( modeBuf[ 0 ]==']'))
    {
      break;
    }else{
      Serial.print(".");    
      delay( 200 );
    }
  }

  Serial.println( "]" );
  
  if( i >= 100 )
  {
    // no comand just Beacon operation
    Serial.println("d[no command from master. So CFG Mode]");
    buf[ 0 ]= opModeCfg;
    buf[ 1 ]= '-';
    buf[ 2 ]= '-';  
  }
  
  if(( buf[ 0 ]== opModeBeacon )
    ||( buf[ 0 ]== opModeBell ))
  {
    // Beacon & Bell operation mode selected
    
    pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
    setup_wifi();

    client.setServer(iotBrockerIP.c_str(), iotMqttPort);
    client.setCallback(callback);
    return;
  }
  else if( buf[ 0 ]== opModeCfg )
  {
    // Configuration mode selected
    WiFi.softAP(ssid);                                                    // Создаём точку доступа
  
    HTTP.begin();                                                         // Инициализируем Web-сервер
    ftpSrv.begin("relay","relay");                                        // Поднимаем FTP-сервер для удобства отладки работы HTML (логин: relay, пароль: relay)

    Serial.println("d[My IP to connect via Web-Browser or FTP:]");        // Выводим на монитор серийного порта сообщение о том, что сейчас будем выводить локальный IP
    Serial.print( "d[" );
    Serial.print( WiFi.softAPIP());                                       // Выводим локальный IP-адрес ESP8266
    Serial.println("]");

    // Обработка HTTP-запросов
/*    
    HTTP.on("/relay_switch", [](){                                        // При HTTP запросе вида http://192.168.4.1/relay_switch
        HTTP.send(200, "text/plain", relay_switch());                     // Отдаём клиенту код успешной обработки запроса, сообщаем, что формат ответа текстовый и возвращаем результат выполнения функции relay_switch 
    });
    HTTP.on("/relay_status", [](){                                        // При HTTP запросе вида http://192.168.4.1/relay_status
       HTTP.send(200, "text/plain", relay_status());                      // Отдаём клиенту код успешной обработки запроса, сообщаем, что формат ответа текстовый и возвращаем результат выполнения функции relay_status 
    });
  */
    HTTP.on("/get_cfg", [](){                                             // При HTTP запросе вида http://192.168.4.1/get_cfg
       HTTP.send(200, "text/plain", get_cfg());                           // Отдаём клиенту код успешной обработки запроса, как payload возвращаем содержимое config файла 
    });
    HTTP.on("/save_cfg", [](){                                             // При HTTP запросе вида http://192.168.4.1/save_cfg
       HTTP.send(200, "text/plain", save_cfg());                            // Отдаём клиенту код успешной обработки запроса, как payload получаем содержимое config файла 
    });
    HTTP.onNotFound([](){                                                 // Описываем действия при событии "Не найдено"
    if(!handleFileRead(HTTP.uri()))                                       // Если функция handleFileRead (описана ниже) возвращает значение false в ответ на поиск файла в файловой системе
        HTTP.send(404, "text/plain", "Not Found");                        // возвращаем на запрос текстовое сообщение "File isn't found" с кодом 404 (не найдено)
    });

    Serial.println("d[READY]");
  }else
  {
    // Unknown mode is on
  }
}

void loop() 
{
  if(( buf[ 0 ]== opModeBeacon )
    ||( buf[ 0 ]== opModeBell ))
  {
      if( !client.connected()) 
      {
        reconnect();
      }
      client.loop();

      if( pubs_number )
      {
        if( buf[ 0 ]== opModeBeacon )
        {
          // build JSON for Beacon message
          snprintf( msg, 128, "{\"%s\":\"%s\",\"Battery\":%c%c}", 
            json_iotDevName, iotDevName.c_str(), buf[ 1 ], buf[ 2 ]);
        }else{
          // build JSON for Bell message
          snprintf (msg, 128, "{\"%s\":\"%s\",\"Battery\":%c%c,\"Bell\":\"On\"}", 
            json_iotDevName, iotDevName.c_str(), buf[ 1 ], buf[ 2 ]);
        } 
        Serial.print( "d[" );
        Serial.print( msg );
        Serial.println( "]" );
        client.publish( iotTopic.c_str(), msg );
        Serial.println( "b[Ok]" );
        pubs_number--; // Decrease number of publications
      }
      return;
  }
  else if( buf[ 0 ]== opModeCfg )
  {
    // Run in cofiguration mode
    ftpSrv.handleFTP();                                                 // Обработчик FTP-соединений  
    HTTP.handleClient();                                                // Обработчик HTTP-событий (отлавливает HTTP-запросы к устройству и обрабатывает их в соответствии с выше описанным алгоритмом)
  }
}

bool handleFileRead(String path){                                       // Функция работы с файловой системой
  if(path.endsWith("/")) path += "index.html";                          // Если устройство вызывается по корневому адресу, то должен вызываться файл index.html (добавляем его в конец адреса)
  String contentType = getContentType(path);                            // С помощью функции getContentType (описана ниже) определяем по типу файла (в адресе обращения) какой заголовок необходимо возвращать по его вызову
  if(SPIFFS.exists(path)){                                              // Если в файловой системе существует файл по адресу обращения
    File file = SPIFFS.open(path, "r");                                 // Открываем файл для чтения
    size_t sent = HTTP.streamFile(file, contentType);                   // Выводим содержимое файла по HTTP, указывая заголовок типа содержимого contentType
    file.close();                                                       // Закрываем файл
    return true;                                                        // Завершаем выполнение функции, возвращая результатом ее исполнения true (истина)
  }
  return false;                                                         // Завершаем выполнение функции, возвращая результатом ее исполнения false (если не обработалось предыдущее условие)
}

String getContentType(String filename){                                 // Функция, возвращающая необходимый заголовок типа содержимого в зависимости от расширения файла
  if (filename.endsWith(".html")) return "text/html";                   // Если файл заканчивается на ".html", то возвращаем заголовок "text/html" и завершаем выполнение функции
  else if (filename.endsWith(".css")) return "text/css";                // Если файл заканчивается на ".css", то возвращаем заголовок "text/css" и завершаем выполнение функции
  else if (filename.endsWith(".js")) return "application/javascript";   // Если файл заканчивается на ".js", то возвращаем заголовок "application/javascript" и завершаем выполнение функции
  else if (filename.endsWith(".png")) return "image/png";               // Если файл заканчивается на ".png", то возвращаем заголовок "image/png" и завершаем выполнение функции
  else if (filename.endsWith(".jpg")) return "image/jpeg";              // Если файл заканчивается на ".jpg", то возвращаем заголовок "image/jpg" и завершаем выполнение функции
  else if (filename.endsWith(".gif")) return "image/gif";               // Если файл заканчивается на ".gif", то возвращаем заголовок "image/gif" и завершаем выполнение функции
  else if (filename.endsWith(".ico")) return "image/x-icon";            // Если файл заканчивается на ".ico", то возвращаем заголовок "image/x-icon" и завершаем выполнение функции
  return "text/plain";                                                  // Если ни один из типов файла не совпал, то считаем что содержимое файла текстовое, отдаем соответствующий заголовок и завершаем выполнение функции
}

bool loadConfig() {
  Serial.println("\nd[CFG:Load start]");
  
  File configFile = SPIFFS.open("/cfg.json", "r");
  if (!configFile) {
    Serial.println("d[CFG:Load cannot open cfg.json]");
    
    if( !createDefaultConfig()){
      Serial.println("d[CFG:Failed to create default config file]");
      return false;  
    }

    Serial.println("d[CFG:Default cfg.json generated]");

    configFile = SPIFFS.open("/cfg.json", "r");
    if( !configFile ){
      Serial.println("d[Failed to open default config file]");
      return false;  
    }
  }

  Serial.println("d[CFG:cfg.json was loaded]");

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("d[Config file size is too large]");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<512> cfg;                                         // Config в виде JSON Array
  auto error = deserializeJson(cfg, buf.get());
  if (error) {
    Serial.println("d[Failed to parse config file]");
    return false;
  }

  const char* pTmp;

  // iotDevName
  pTmp              = cfg[json_iotDevName];
  iotDevName        = String( pTmp );
  
  // iotBrockerIP
  pTmp              = cfg[json_iotBrockerIP];
  iotBrockerIP      = String( pTmp );

  // iotBrockerKey
  pTmp              = cfg[json_iotBrockerKey];
  iotBrockerKey     = String( pTmp );

  // iotMqttPort  
  iotMqttPort       = cfg[json_iotMqttPort];
  
  // iotBeaconTO should be uint
  iotBeaconTO       = cfg[json_iotBeaconTO];
  
  // iotWiFiSsid
  pTmp              = cfg[json_iotWiFiSsid];
  iotWiFiSsid       = String( pTmp );
  
  // iotWiFiKey
  pTmp              = cfg[json_iotWiFiKey];
  iotWiFiKey        = String( pTmp );

  // iotTopic
  pTmp              = cfg[json_iotTopic];
  iotTopic          = String( pTmp );

  // iotSsl
  pTmp              = cfg[json_iotSsl];
  iotSsl            = String( pTmp );

  configFile.close();
  return true;
}

bool saveConfig() 
{
  StaticJsonDocument<512> cfg;

  cfg[json_iotDevName]    = iotDevName;
  cfg[json_iotBrockerIP]  = iotBrockerIP;
  cfg[json_iotBrockerKey] = iotBrockerKey;
  cfg[json_iotMqttPort]   = iotMqttPort;
  cfg[json_iotBeaconTO]   = iotBeaconTO;
  cfg[json_iotWiFiSsid]   = iotWiFiSsid;
  cfg[json_iotWiFiKey]    = iotWiFiKey;
  cfg[json_iotTopic]      = iotTopic;
  cfg[json_iotSsl]        = iotSsl;
  
  File configFile = SPIFFS.open("/cfg.json", "w");
  if (!configFile) {
    Serial.println("d[Failed to open config file for writing]");
    return false;
  }
  serializeJson( cfg, configFile );

  configFile.close();
  return true;
}

bool createDefaultConfig()
{
  StaticJsonDocument<512> doc;
  doc[json_iotDevName]    = def_cfg_iotDevName;       // Device Name like "Lamp Controler" 
  doc[json_iotBrockerIP]  = def_cfg_iotBrockerIP;     // MQTT Brocker IP if uses
  doc[json_iotBrockerKey] = def_cfg_iotBrockerKey;    // Password to MQTT 
  doc[json_iotMqttPort]   = def_cfg_iotMqttPort;      // MQTT Broker Port number to send beacon ping 
  doc[json_iotBeaconTO]   = def_cfg_iotBeaconTO;      // Time to send beacon ping 
  doc[json_iotWiFiSsid]   = def_cfg_iotWiFiSsid;      // WiFi Router IP
  doc[json_iotWiFiKey]    = def_cfg_iotWiFiKey;       // WiFi Router Password
  doc[json_iotTopic]      = def_cfg_iotTopic;         // MQTT topic to send state & bell
  doc[json_iotSsl]        = def_cfg_iotSsl;           // Use SSL for MQTT sessions

  File configFile = SPIFFS.open("/cfg.json", "w");
  if (!configFile) 
    return false;

  serializeJson(doc, configFile);

  configFile.close();
  return true;
}

String get_cfg()
{
  String cfg = String(""); 
  File configFile = SPIFFS.open("/cfg.json", "r");
  if( !configFile ){
    return cfg;  
  }

  Serial.println("\nd[");
 
  while(configFile.available()){
      cfg +=(char)configFile.read();
  }
  Serial.println( cfg );
  Serial.println( "]" );
 
  configFile.close();
  return cfg;
}

String save_cfg()
{
  String jsonConfig = "" +HTTP.arg("plain");
  Serial.println( "d[POST body was:]\nd:[" );
  Serial.println( jsonConfig );
  Serial.println( "]" );

  File configFile = SPIFFS.open("/cfg.json", "w");
  if (!configFile) {
    Serial.println("d[Failed to open config file for writing]");
    return "Error-cfg file not opened";
  }
  configFile.print( jsonConfig );
  configFile.close();
 
  return jsonConfig;
}

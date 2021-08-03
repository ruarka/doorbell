// Data Sending Proto by MC in JSON like format
// MC JSON PROTO to deliver API between Master Controller(MC) implements main application logic  
// and WiFi transmition part based on ESP8266.
// There is no strict binding to EPS but it is the 1'st chanel creation device used
// 
// MESSAGE  - JSON formated string. The number of opened('{') & closed('}') curly brackets should be the same.
//            The message should be finished by 0x0D or 0x0A.
//            When condition "message is correct" met then the message is submited/transmited to MC by
//            call of "mc_json_proto_DO" in loop routine. 
// 
// {!Correct this} INCOMING - The number of bytes sending by "mc_json_proto_puts" or "mc_json_proto_putc". Proto takes into 
//            account the message could be sent partly. It means that the number byte received from Serial
//            is sending by "mc_json_proto_puts" or "mc_json_proto_putc" without counstruction hole message.
//                        

// От MC могут 
// j : json
// p : debug
// 
// От ESP к МС
// с : конфигурация
// j : json
// p : debug
// 
// j:{"soil_moisture":[1111,2222,3333,4444,5555]}
//
// {"msgt":"c","msg":"\"soil_moisture\":[1111,2222,3333,4444,5555]"}

#define IN_BUFFER_SIZE      256     // From MC buffer size
#define OUT_BUFFER_SIZE     256     // To MC message buffer size

// int   iNumberOfBrackets = 0;         // Counter of opened('{') & closed('}') curly brackets
char  pInBuffer[ IN_BUFFER_SIZE+1 ];    // +1 to add 0x00 on final stage
int   iInBuffer = 0;                    // Number of bytes in IN Buffer
char  pOutBuffer[ OUT_BUFFER_SIZE+1 ];  // +1 to add 0x00 on final stage
int   iOutBuffer = 0;                   // Number of bytes in OUT Buffer

bool mc_json_proto_init()
{
  iInBuffer = 0;
//  iNumberOfBrackets = 0;
  iOutBuffer = 0;
  
  return true;
}

int mc_json_proto_putc( int c )
{
  Serial.write(c);
  return 0;  
}

int mc_json_proto_puts( String s )
{
  Serial.println( s );
  
  return 0;  
}

// Как только набралось сообщение, то отправляем
// Функция должна вызываться в loop
bool mc_json_proto_DO()
{
  int incomingByte = 0;
  
  while( Serial.available() > 0) {
    incomingByte = Serial.read();

    

    switch((char)incomingByte ){
      case 0x0D:
      case 0x0A:
          if( iInBuffer ){
            // TODO: Send message to WiFi in async mode
            StaticJsonDocument<200> doc;

            if( pInBuffer[ iInBuffer-1 ]!= 0x00 )
              pInBuffer[ iInBuffer ]= 0x00;

            Serial.print( "S:JSON-" );
            Serial.println( pInBuffer );
            
            // Deserialize the JSON document
            DeserializationError error = deserializeJson(doc, pInBuffer);
            
           // Test if parsing succeeds
            if( error ){
              // Release buffer
              iInBuffer = 0;

              Serial.print( "MP:Err-" );
              Serial.println( pInBuffer );
              
              return false;
            }

            // check received message [msgt==c] to send
            if( doc["msgt"].as<String>().equals("c"))
            {
              if(WiFi.status()== WL_CONNECTED){
                WiFiClient client;
                HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://jigsaw.w3.org/HTTP/connection.html")) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    }
    // Free resources
    http.end();

                 Serial.print("[HTTP] POST begin...\n");
                if(!http.begin(client, "http://10.1.30.43/index.html")) 
                {
                  http.end();
                  
                  // Release buffer
                  iInBuffer = 0;

                  Serial.print("[HTTP] POST ERR...\n");
                      
                  return true;
                }

                iotBrockerIP = "10.1.30.43";
                
                Serial.print( "MP:1-");
                Serial.println( iotBrockerIP );

                char serverName[ sizeof( iotBrockerIP )];
                iotBrockerIP.toCharArray( serverName, sizeof( iotBrockerIP ));

                // Your Domain name with URL path or IP address with path
                Serial.print( "MP:2-" );
                Serial.println( serverName );
          //      http.begin(serverName);

                // If you need an HTTP request with a content type: application/json, use the following:
         //       http.addHeader("Content-Type", "application/json");

                char pPost[ 200 ];
                String sTmp = doc["msg"];
                sTmp.toCharArray( pPost, sizeof( sTmp ));

                Serial.print( "MP:3-" );
                Serial.println( pPost );
                
                // int httpResponseCode = http.POST( pPost );
                int httpResponseCode = http.GET();

                Serial.print("HTTP Response code: ");
                Serial.println( httpResponseCode );

                // Free resources
                http.end();
              }
              else
                return false;
            }
                  
            // Release buffer
            iInBuffer = 0;

            return true;
          }else
            return true;
      
      default:
        if( iInBuffer < IN_BUFFER_SIZE ){
          pInBuffer[ iInBuffer++ ]= (char)incomingByte;
        }else{
          // Clear IN buffer because of overflow
          iInBuffer = 0;
          return false;
        }
       break;
    }
  }
  return true;
}

// Debug print
void printDbgMsg( String msg )
{
  
}

  #include <Wire.h>
  #include <Adafruit_ADS1015.h>
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
  
  // Change the credentials below, so your ESP8266 connects to your router
  const char* ssid = "YOUR-WIFI";
  const char* password = "WIFI-PASSWORD";
  
  // Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
  const char* mqtt_server = "xxx.xxx.xxx.xxx";  //MQTT Server IP Address
  const int mqtt_server_port = 1883;  //MQTT Server Port
  const char* mqtt_client = "WindGenerator";  //MUST BE UNIQUE ON THE MQTT SERVER
  const char* topic1 = "sensors/WindGenerator/rawAmps";   //Topics on MQTT change if needed
  
  
  // Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
  WiFiClient espClient;
  PubSubClient client(espClient);
  
  //Setup the ADS1115 for 16nit resolution
  Adafruit_ADS1115 ads;  

  // Timers auxiliar variables
  long now = millis();
  long lastMeasure = 0;

  void setup(void)
  {
    Serial.begin(115200);
    
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Inizialize ADS at 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
    ads.setGain(GAIN_SIXTEEN);    
    ads.begin();

    setup_wifi();
    client.setServer(mqtt_server, mqtt_server_port);
  }
  
  void loop(void)
  {
    // Check if MQQT Is connected
    if (!client.connected()) {
      reconnect();
    }
    if(!client.loop()) {
      client.connect(mqtt_client);
    }
    now = millis();
    // Publishes new data to MQTT every 1 seconds
    if (now - lastMeasure > 1000) {
      lastMeasure = now;
    
      // Read the ADS1115
      int16_t results;
      results = ads.readADC_Differential_0_1();  
   
      Serial.print("Amps: "); 
      
      // Calc the Amps - Formula work only for 100A/100mV SHUNT UNCOMMENT WHERE NECESSARY
      float amps = ((float)results * 256.0) / 32768.0;//100mv shunt
      //amps = amps * 1.333; //uncomment for 75mv shunt
      //amps = amps * 2; //uncomment for 50mv shunt
      //amps = amps / 2; //Uncomment for 50A shunt
      //amps = amps / 4; //Uncomment for 25A shunt
      
      Serial.println(amps); 
    
      // Publish Amps Value on MQTT
      static char ampsTemp[7];
      dtostrf(amps, 6, 2, ampsTemp);
      client.publish(topic1, ampsTemp);
      // when publish builtin led blink for 150ms
      // digitalWrite(LED_BUILTIN, LOW);   // Arduino: turn the LED on (HIGH)
      // delay(150);                       // wait for a second
      // digitalWrite(LED_BUILTIN, HIGH);    // Arduino: turn the LED off (LOW)
    }
  }

  // Don't change the function below. This function connects your ESP8266 to your router
  void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      //delay(500);
      Serial.print(".");
      // BLINK builtin led until connect to WIFI
      digitalWrite(LED_BUILTIN, HIGH);   
      delay(250);                       
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);   
    }
    Serial.println("");
    Serial.print("WiFi connected - ESP IP address: ");
    Serial.println(WiFi.localIP());
    
    // Wait 5 seconds then switch off the builtin led
    delay(5000);   
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
  }

// This functions reconnects the ESP8266 to MQTT broker
  void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect(mqtt_client)) {
        Serial.println("connected");  
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
}


  
  
  

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <string>

// Define server pins
#define PIN_AP 5

#define FORM      "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>RU Setup</title><style> *,::after,::before{box-sizing:border-box}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da}button{cursor:pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto}h1{text-align:center}</style></head><body><main class='form-signin'><form action='/' method='post'><h1 class=''>RU Settings</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'></div><div class='form-floating'><br/><label>Password</label><input type='text' class='form-control' name='password'></div><div class='form-floating'><br/><label>RU Tag</label><input type='text' class='form-control' name='ru_tag'></div><div class='form-floating'><br/><label>Ping Topic</label><input type='text' class='form-control' name='ping_topic'></div><div class='form-floating'><br/><label>RFID Topic</label><input type='text' class='form-control' name='rfid_topic'></div><div class='form-floating'><br/><label>Drop sensor ideal status</label><input type='number' class='form-control' name='status'></div><div class='form-floating'><br/><label>Mag and Spring Configuration</label><br/><br/><label>Spring in mag 1:</label><input type='number' class='form-control' name='mag_1'><label>Spring in mag 2:</label><input type='number' class='form-control' name='mag_2'><label>Spring in mag 3:</label><input type='number' class='form-control' name='mag_3'><label>Spring in mag 4:</label><input type='number' class='form-control' name='mag_4'><label>Spring in mag 5:</label><input type='number' class='form-control' name='mag_5'><label>Spring in mag 6:</label><input type='number' class='form-control' name='mag_6'><label>Spring in mag 7:</label><input type='number' class='form-control' name='mag_7'><label>Spring in mag 8:</label><input type='number' class='form-control' name='mag_8'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'></p></form></main></body></html>"
#define RESPONSE  "<!doctype html> <html lang='en'> <head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width, initial-scale=1'> <title>RU Setup</title> <style> *,::after,::before { box-sizing: border-box; } body { margin: 0; font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans'; font-size: 1rem; font-weight: 400; line-height: 1.5; color: #212529; background-color: #f5f5f5; } .form-control { display: block; width: 100%; height: calc(1.5em + .75rem + 2px); border: 1px solid #ced4da; } button { border: 1px solid transparent; color: #fff; background-color: #007bff; border-color: #007bff; padding: .5rem 1rem; font-size: 1.25rem; line-height: 1.5; border-radius: .3rem; width: 100%; } .form-signin { width: 100%; max-width: 400px; padding: 15px; margin: auto; } h1, p { text-align: center; } </style> </head> <body> <main class='form-signin'> <h1>RU Setup</h1> <br/> <p>Settings have been saved successfully...!<br />Restarting the device.</p> </main> </body> </html>"

const char *ap_ssid = "Vertical_RU";     // ssid of your wifi
const char *ap_password = "letmeintoRU"; // wifi password

String ssid;     // ssid of your wifi
String password; // wifi password

String ru_tag;     // Unique RU tag. Source: Dash.vertical-innovations.com
String PING_TOPIC; // Ping topic: || 23jyotipingtran for dhaka || 19jyotipingtran for CTG || 52jyotipingtran for Narayanganj
String RFID_TOPIC; // For LIVE SERVER

unsigned int drop_sensor_ideal_status;
unsigned int drop_sensor_active_status;

String mag;

float mag_spring_conf[9] = {
    0.0, // 0 index is ignored
    1.0, // Number of springs in magazine 1
    1.0, // Number of springs in magazine 2
    1.0, // Number of springs in magazine 3
    1.0, // Number of springs in magazine 4
    1.0, // Number of springs in magazine 5
    1.0, // Number of springs in magazine 6
    1.0, // Number of springs in magazine 7
    1.0  // Number of springs in magazine 8
};

WebServer server(80);

void loadConfig()
{
  // Checking if the configuration directory exist or not
  if (!SPIFFS.exists("/config.json")) {
    Serial.println("Directory does not exist");
    return;
  } else {
    Serial.println("Directory exist\nReadng Configuration");
  }
  // Read the configuration data from the file and deserialize it
  File readFile = SPIFFS.open("/config.json", FILE_READ);
  if (!readFile)
  {
    Serial.println("Failed to open config file for reading");
    return;
  }
  DynamicJsonDocument readJson(1024);
  DeserializationError error = deserializeJson(readJson, readFile);
  if (error)
  {
    Serial.println("Failed to deserialize config data");
    return;
  }
  readFile.close();

  // Extract the values from the deserialized JSON object
  ssid = readJson["ssid"].as<String>();
  password = readJson["password"].as<String>();
  ru_tag = readJson["ru_tag"].as<String>();
  PING_TOPIC = readJson["ping_topic"].as<String>();
  RFID_TOPIC = readJson["rfid_topic"].as<String>();
  drop_sensor_ideal_status = readJson["status"].as<int>();
  drop_sensor_active_status = (!drop_sensor_ideal_status);
  mag_spring_conf[1] = readJson["mag_1"].as<float>();
  mag_spring_conf[2] = readJson["mag_2"].as<float>();
  mag_spring_conf[3] = readJson["mag_3"].as<float>();
  mag_spring_conf[4] = readJson["mag_4"].as<float>();
  mag_spring_conf[5] = readJson["mag_5"].as<float>();
  mag_spring_conf[6] = readJson["mag_6"].as<float>();
  mag_spring_conf[7] = readJson["mag_7"].as<float>();
  mag_spring_conf[8] = readJson["mag_8"].as<float>();
}

void handlePortal()
{
  if (server.method() == HTTP_POST)
  {
    Serial.println("Copying the data into spiffs");
    // Create and populate a JSON object with configuration data in handler
    DynamicJsonDocument configJson(1024);
    configJson["ssid"] = server.arg("ssid");
    configJson["password"] = server.arg("password");
    configJson["ru_tag"] = server.arg("ru_tag");
    configJson["ping_topic"] = server.arg("ping_topic");
    configJson["rfid_topic"] = server.arg("rfid_topic");
    configJson["status"] = server.arg("status");
    configJson["mag_1"] = server.arg("mag_1");
    configJson["mag_2"] = server.arg("mag_2");
    configJson["mag_3"] = server.arg("mag_3");
    configJson["mag_4"] = server.arg("mag_4");
    configJson["mag_5"] = server.arg("mag_5");
    configJson["mag_6"] = server.arg("mag_6");
    configJson["mag_7"] = server.arg("mag_7");
    configJson["mag_8"] = server.arg("mag_8");

    // Serialize the JSON object to a String
    String configData;
    serializeJson(configJson, configData);
    Serial.println("Config data:");
    Serial.println(configData);

    // Save the JSON object to a file in the SPIFFS
    File configFile = SPIFFS.open("/config.json", FILE_WRITE);
    if (!configFile)
    {
      Serial.println("Failed to open config file for writing");
      server.send(500, "text/plain", "Failed to open config file for writing");
      return;
    }
    serializeJson(configJson, configFile);
    configFile.close();

    // Sending web response html file
    server.send(200, "text/html", RESPONSE);
    Serial.println("Settings saved to SPIFFS restarting device");
    delay(1000);
    server.stop();
    ESP.restart();
  }
  else
  {
    // Sending the html from to get user data
    server.send(200, "text/html", FORM);
  }
}

void handleNotFound()
{
  server.send(404, "text/plain", "File Not Found\n\n");
}

inline void StartAP(void)
{
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  delay(1000);
  WiFi.mode(WIFI_AP_STA);
  IPAddress AP_LOCAL_IP(192, 168, 1, 160);
  IPAddress AP_GATEWAY_IP(192, 168, 1, 4);
  IPAddress AP_NETWORK_MASK(255, 255, 255, 0);
  if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK))
  {
    Serial.println("AP Config Failed");
    return;
  }
  delay(1000);
  if (!WiFi.softAP(ap_ssid, ap_password))
  {
    Serial.println("AP Failed");
    return;
  }
  Serial.println("Initializing softap");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handlePortal);
  server.onNotFound(handleNotFound);
  server.begin();
  while (server.method() != HTTP_POST)
  {
    server.handleClient();
  }
}

inline void OnDemandAP(){
  int count = 0;
  while (!digitalRead(PIN_AP))
  {
    count++;
    Serial.println("Button count:" + String(count));
    Serial.println(count);
    if (count == 15)
    {
      count = 0;
      StartAP();
    }
    delay(200);
  }
  count = 0;
}

void setup()
{
  Serial.begin(115200); // Initialising if(DEBUG)Serial Monitor
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  pinMode(PIN_AP, INPUT_PULLUP);

  loadConfig();

  if (ssid.length() == 0 && password.length() == 0)
  {
    Serial.println("Configuration File not found \n Starting Portal");
    StartAP();
  }
  else
  {
    Serial.println("got credentials from spiffs");
  }

  Serial.println("Connecting to WiFi..!");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(ssid);
    Serial.println(password);
  }
  Serial.println("connected");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("connected..!" + String(digitalRead(PIN_AP)));
    Serial.println(PING_TOPIC);
    Serial.println(RFID_TOPIC);
    Serial.println(ru_tag);
    Serial.println(drop_sensor_ideal_status);
    Serial.println(drop_sensor_active_status);
    Serial.println(mag_spring_conf[1]);
    Serial.println(mag_spring_conf[2]);
    Serial.println(mag_spring_conf[3]);
    Serial.println(mag_spring_conf[4]);
    Serial.println(mag_spring_conf[5]);
    Serial.println(mag_spring_conf[6]);
    Serial.println(mag_spring_conf[7]);
    Serial.println(mag_spring_conf[8]);
  }
  delay(1000);
  OnDemandAP();
}

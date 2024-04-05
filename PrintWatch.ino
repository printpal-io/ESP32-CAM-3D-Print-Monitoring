#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <iostream>
#include <string>
#include "esp32cam.h"
#include <esp_camera.h>
#include "Base64.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

esp32cam::Resolution initialResolution;
esp32cam::Resolution currentResolution;


const char* WIFI_SSID = "ENTER_SSID";
const char* WIFI_PASS = "ENTER_PASSWORD";
const char *API_KEY = "ENTER_API_KEY";
const char *PRINTER_ID = "enter-custom-printer-id-name";

const char *ai_url = "https://ai.printpal.io/api/v2/infer";

long lastTime = millis();

WiFiClientSecure client;
HTTPClient https;

void runInfer()
  {
    if (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        WiFi.disconnect();
        WiFi.reconnect();
        Serial.println("TASK DELETED BECAUSE WIFI DOWN");
        delay(2000);    
    } else {
      String encodedImage;

      camera_fb_t* fb = esp_camera_fb_get();
      if (fb != nullptr) {
        //buff->fb;
        encodedImage = base64::encode((uint8_t *)fb->buf, fb->len);
      } 
      Serial.println("SIZEOF BUF: ");
      Serial.println(fb->len);

      DynamicJsonDocument doc(768);

      float c_thresholds[2] = {0.30, 0.60};
      JsonArray thresholds = doc.createNestedArray("thresholds");
      for (float i: c_thresholds)  {
        thresholds.add(i);
      }

      float c_scores[16] = {0.0, 0.10, 0.10, 0.20, 0.01, 0.02, 0.03, 0.05, 0.0, 0.1, 0.01, 0.04, 0.03, 0.02, 0.01};
      JsonArray scores = doc.createNestedArray("scores");
      for (float i: c_scores)  {
        scores.add(i);
      }

      doc["api_key"] = API_KEY;
      doc["printer_id"] = PRINTER_ID;
      doc["state"] = 0;
      doc["version"] = "1.2.11";
      doc["ticket_id"] = "";
      doc["buffer_length"] = 16;
      doc["conf"] = 0.60;
      doc["buffer_percent"] = 80;
      doc["printTime"] = 522.2;
      doc["printTimeLeft"] = 2.2;
      doc["progress"] = 3.3;
      doc["job_name"] = "esp32-cam.stl";
      doc["sma_spaghetti"] = 0.15;
      doc["enable_feedback_images"] = true;

      String requestBody;
      serializeJson(doc, requestBody);
      Serial.println(requestBody);
      int idx = requestBody.lastIndexOf("}");
      requestBody = requestBody.substring(0, idx) + ",\"image_array\": \"" + encodedImage.c_str() + "\"}";
      Serial.println(requestBody);
      Serial.println("DONE");
      delay(2000);

      https.begin(client, ai_url);
      https.addHeader("Content-Type", "application/json; charset=UTF-8");        

      int httpCode = https.sendRequest("POST", requestBody); // we simply put the whole image in the post body.
      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
          String payload = https.getString();
          Serial.println("RESPONSE SUCCESS: ");
          Serial.println(payload);

        } else {
          Serial.println("We ran into a bit of an issue...");
          Serial.println(https.getString());
        }
      }
      else
      {
        Serial.printf("[HTTP] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        Serial.println("FAILED");
      }
      esp_camera_fb_return(fb);
      https.end();

    }

  }

void runInferLoop(void* ctx) {
  while (true) {
    runInfer();
    delay(12000);
  }
  vTaskDelete( NULL );
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  Serial.println();
  delay(2000);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi failure");
    delay(5000);
    ESP.restart();
  }
  Serial.println("WiFi connected");

  {
    using namespace esp32cam;

    initialResolution = Resolution::find(640, 480);
    currentResolution = initialResolution;

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);

    cfg.setJpeg(90);
    cfg.setBufferCount(2);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }

  Serial.println("camera starting");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  client.setTimeout(10000);
  client.setInsecure();
  https.setTimeout(10000);
  xTaskCreatePinnedToCore(runInferLoop, "printwatch-acquire", 8192, NULL, 1, NULL, 0);
}

void loop() {
  // do nothing
  delay(1);
}

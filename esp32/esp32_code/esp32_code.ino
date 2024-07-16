#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

// Camera model: AI Thinker
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// WiFi credentials
const char* ssid = "Test";
const char* password = "Welcome@123";

// Server URL
const char* serverUrl = "http://192.168.29.219:5000/uploadProfilePicture";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
}

void loop() {
  // Capture picture
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Send picture to server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    String payload = "--" + boundary + "\r\n" +
                     "Content-Disposition: form-data; name=\"mypic\"; filename=\"capture.jpg\"\r\n" +
                     "Content-Type: image/jpeg\r\n\r\n";

    // Calculate the total length of the request body
    int contentLength = payload.length() + fb->len + 2 + boundary.length() + 6;
    
    // Send the Content-Length header
    http.addHeader("Content-Length", String(contentLength));

    // Send the POST request
    int httpResponseCode = http.sendRequest("POST", (uint8_t *)payload.c_str(), payload.length());

    // Send the image data
    int bytesSent = http.write(fb->buf, fb->len);
    if (bytesSent != fb->len) {
      Serial.printf("Error sending data: only %d/%d bytes sent\n", bytesSent, fb->len);
    }

    // Send the end boundary
    String endPayload = "\r\n--" + boundary + "--\r\n";
    http.print(endPayload);

    // Read the response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.printf("Error code: %d\n", httpResponseCode);
    }

    http.end();
  }

  // Return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);

  // Delay before capturing the next image
  delay(5000);
}

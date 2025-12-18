// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ElegantOTA.h>
#include <HardwareSerial.h>
#include <MycilaESPConnect.h>
#include <MycilaWebSerial.h>
#include <StreamString.h>
#include <WiFi.h>

#define TAG "DataLogger"

static AsyncWebServer server(80);
static WebSerial webSerial;
static Mycila::ESPConnect espConnect(server);
static StreamString buffer;

static void startNetworkServices() {
  server.onNotFound([](AsyncWebServerRequest* request) { request->redirect("/webserial"); });
  server.begin();
  ESP_LOGI(TAG, "Enable mDNS");
  MDNS.begin(espConnect.getConfig().hostname.c_str());
  MDNS.addService("http", "tcp", 80);
}

void setup() {
  Serial.begin(115200);

#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  // Logging

  esp_log_level_set("*", ESP_LOG_DEBUG);
  esp_log_level_set("esp_core_dump_elf", ESP_LOG_INFO);
  esp_log_level_set("esp_core_dump_port", ESP_LOG_INFO);
  esp_log_level_set("esp_netif_lwip", ESP_LOG_INFO);
  esp_log_level_set("nvs", ESP_LOG_INFO);
  esp_log_level_set("ARDUINO", ESP_LOG_DEBUG);
  esp_log_level_set(TAG, ESP_LOG_DEBUG);
  ESP_LOGI(TAG, "Logging initialized");

  // Connect to remote ESP32 RX0/TX0 pins

  ESP_LOGI(TAG, "Starting Serial connection to target device: RX=%d, TX=%d", MYCILA_DATA_LOGGER_RX, MYCILA_DATA_LOGGER_TX);
  MYCILA_DATA_LOGGER_SERIAL.begin(115200, SERIAL_8N1, MYCILA_DATA_LOGGER_RX, MYCILA_DATA_LOGGER_TX);

  // Factory reset and reboot

  server.on("/reset", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest* request) {
    ESP_LOGW(TAG, "Factory reset requested via /reset");
    espConnect.clearConfiguration();
    ESP.restart();
    request->send(200);
  });

  // Restart

  server.on("/restart", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest* request) {
    ESP_LOGW(TAG, "Restart requested via /restart");
    ESP.restart();
    request->send(200);
  });

  // OTA Updates

  ElegantOTA.setAutoReboot(true);
  ElegantOTA.begin(&server);

  // Web Serial

  webSerial.begin(&server);

  // Network Manager

  espConnect.setAutoRestart(true);
  espConnect.setBlocking(false);
  espConnect.listen([](Mycila::ESPConnect::State previous, Mycila::ESPConnect::State state) {
    ESP_LOGD(TAG, "NetworkState: %s => %s", espConnect.getStateName(previous), espConnect.getStateName(state));
    switch (state) {
      case Mycila::ESPConnect::State::NETWORK_DISABLED:
        ESP_LOGW(TAG, "Disabled Network!");
        break;
      case Mycila::ESPConnect::State::AP_STARTING:
        ESP_LOGI(TAG, "Starting Access Point %s...", espConnect.getAccessPointSSID().c_str());
        break;
      case Mycila::ESPConnect::State::AP_STARTED:
        ESP_LOGI(TAG, "Access Point %s started with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        startNetworkServices();
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTING:
        ESP_LOGI(TAG, "Connecting to network...");
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTED:
        ESP_LOGI(TAG, "Connected with IP address %s", espConnect.getIPAddress().toString().c_str());
        startNetworkServices();
        break;
      case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
        ESP_LOGW(TAG, "Unable to connect!");
        break;
      case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
        ESP_LOGW(TAG, "Disconnected!");
        break;
      case Mycila::ESPConnect::State::NETWORK_RECONNECTING:
        ESP_LOGI(TAG, "Trying to reconnect...");
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTING:
        ESP_LOGI(TAG, "Starting Captive Portal %s for %" PRIu32 " seconds...", espConnect.getAccessPointSSID().c_str(), espConnect.getCaptivePortalTimeout());
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTED:
        ESP_LOGI(TAG, "Captive Portal started at %s with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        break;
      case Mycila::ESPConnect::State::PORTAL_COMPLETE: {
        if (espConnect.getConfig().apMode) {
          ESP_LOGI(TAG, "Captive Portal: Access Point configured");
        } else {
          ESP_LOGI(TAG, "Captive Portal: WiFi configured");
        }
        break;
      }
      case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
        ESP_LOGW(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
  });
  espConnect.begin("DataLogger", "DataLogger");
}

void loop() {
  espConnect.loop();
  ElegantOTA.loop();

  while (MYCILA_DATA_LOGGER_SERIAL.available()) {
    int c = MYCILA_DATA_LOGGER_SERIAL.read();

    // EOF
    if (c == -1)
      break; // break the while loop to avoid stalling the loop()

    // EOL
    if (c == '\n') {
      size_t len = buffer.length();

      // Remove trailing \r
      if (len && buffer.charAt(len - 1) == '\r')
        len--;

      if (len) {
        // Print the line to local Serial monitor
        Serial.print("[REMOTE] > ");
        Serial.write((const uint8_t*)buffer.c_str(), len);
        Serial.println();

        // Send the line to all connected WebSerial clients
        AsyncWebSocketMessageBuffer* message = webSerial.makeBuffer(len);
        if (message != nullptr) {
          memcpy(message->get(), buffer.c_str(), len);
          webSerial.send(message);
        }

        // Clear the buffer
        buffer.clear();
      }

      break; // break the while loop to avoid stalling the loop()
    }

    // Add character to buffer
    buffer.write((uint8_t)c);
  }
}

/*
 * @author Duy Tran
 * @date 2024-11-08
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include "./web_content.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
HTTPClient http;
WiFiClient client;
MDNSResponder mdns;
ESP8266WebServer server(80);

String baseURL  = "http://172.20.10.5/";    // Modify it with your Network IPv4
String WiFiName = "YourWiFiName";           // Wi-Fi name for connection
String WiFiPW   = "YourWiFiPassword";       // Wi-Fi password for connection

/***********************************************************************************************************************
 * Function declarations
 **********************************************************************************************************************/
static void mainPage();
static void controlPage();
static void contactPage();
static void faqPage();
static void sendPostRequest(String status, String db_num_php);
static void sendPageContent(String content);

#endif // WEBSERVER_H

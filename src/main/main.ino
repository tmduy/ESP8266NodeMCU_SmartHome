/*
 * @author Duy Tran
 * @date 2024-11-08
 */

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "./includes/IOControl.h"
#include "./includes/WebServer.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CREATE_ROUTE_HANDLER(handlerName, pageContent)  \
        static void handlerName()                       \
        {                                               \
            sendPageContent(pageContent);               \
        }

/***********************************************************************************************************************
 * Route Handlers - Define page routes for server
 **********************************************************************************************************************/
CREATE_ROUTE_HANDLER(mainPage, homePageContent);
CREATE_ROUTE_HANDLER(controlPage, controlPageContent);
CREATE_ROUTE_HANDLER(contactPage, contactPageContent);
CREATE_ROUTE_HANDLER(faqPage, faqPageContent);

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
String led1;
String led2;
String ringStatus;
String tempValue;
String gasStatus;

/***********************************************************************************************************************
 * Function Definitions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Sends the HTML content to the client.
 * 
 * This function is called to send a page's HTML content to the client. It responds with a 200 status code and the
 * provided HTML content.
 * 
 * @param content The HTML content to be sent to the client.
 **********************************************************************************************************************/
static void sendPageContent(String content)
{
    server.send(200, "text/html", content);
}

/*******************************************************************************************************************//**
 * @brief Sends a POST request to update data on a PHP server.
 * 
 * This function sends a POST request to the PHP server with the provided status and database endpoint. It sends the
 * status as part of the form data and logs the response from the server.
 * 
 * @param status The status to be sent (e.g., "ON", "OFF", "ERROR").
 * @param db_num_php The endpoint of the PHP script for handling the data insertion.
 **********************************************************************************************************************/
static void sendPostRequest(String status, String db_num_php)
{
    http.begin(client, baseURL + db_num_php);                               // Construct the correct URL
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    // Specify content-type header
    http.POST("status=" + status);                                          // Send the request
    String payload = http.getString();                                      // Get the response payload
    Serial.println(status);                                                 // Print status for debugging
}

/*******************************************************************************************************************//**
 * @brief Handles turning on a device (relay) based on sensor state.
 * 
 * This function checks the state of the specified sensor before and after toggling the relay. If the state changes
 * correctly, it updates the status and sends it to the PHP server.
 * 
 * @param sensorPin The pin where the sensor is connected.
 * @param relayPin The pin controlling the relay.
 * @param data A reference to the variable holding the status (ON/OFF/ERROR).
 * @param dbEndpoint The endpoint for sending data to the PHP server.
 **********************************************************************************************************************/
static void turnON_handler(int sensorPin, int relayPin, String& data, String dbEndpoint)
{
    int sensorStateBefore = digitalRead(sensorPin);

    delay(100);

    digitalWrite(relayPin, LOW);

    server.send(200, "text/html", controlPageContent);

    delay(100);

    int sensorStateAfter = digitalRead(sensorPin);

    if ((sensorStateBefore == 1 && sensorStateAfter == 0) || (sensorStateBefore == 0 && sensorStateAfter == 0))
    {
        data = "ON";
        sendPostRequest(data, dbEndpoint);
    }
    else if ((sensorStateBefore == 1 && sensorStateAfter == 1) || (sensorStateBefore == 0 && sensorStateAfter == 1))
    {
        data = "ERROR";
        sendPostRequest(data, dbEndpoint);
    }
}

/*******************************************************************************************************************//**
 * @brief Handles turning off a device (relay) based on sensor state.
 * 
 * This function checks the state of the specified sensor before and after toggling the relay. If the state changes
 * correctly, it updates the status and sends it to the PHP server.
 * 
 * @param sensorPin The pin where the sensor is connected.
 * @param relayPin The pin controlling the relay.
 * @param data A reference to the variable holding the status (ON/OFF/ERROR).
 * @param dbEndpoint The endpoint for sending data to the PHP server.
 **********************************************************************************************************************/
static void turnOFF_handler(int sensorPin, int relayPin, String& data, String dbEndpoint)
{
    int sensorStateBefore = digitalRead(sensorPin);

    delay(100);

    digitalWrite(relayPin, HIGH);

    server.send(200, "text/html", controlPageContent);

    delay(100);

    int sensorStateAfter = digitalRead(sensorPin);

    if ((sensorStateBefore == 0 && sensorStateAfter == 1) || (sensorStateBefore == 1 && sensorStateAfter == 1))
    {
        data = "OFF";
        sendPostRequest(data, dbEndpoint);
    }
    else if ((sensorStateBefore == 0 && sensorStateAfter == 0) || (sensorStateBefore == 1 && sensorStateAfter == 0))
    {
        data = "ERROR";
        sendPostRequest(data, dbEndpoint);
    }
}

/*******************************************************************************************************************//**
 * @brief Reads the temperature from the DHT sensor.
 * 
 * This function reads the temperature from the DHT sensor. If the value is valid, it updates the corresponding data
 * variable with the temperature value.
 **********************************************************************************************************************/
static void readTemp()
{
    float temp = dht.readTemperature();

    if (String(temp) != "nan")
    {
        tempValue = temp;
    }
}

/*******************************************************************************************************************//**
 * @brief Collects data from sensors, updates relay states, and sends a JSON response.
 * 
 * This function reads the temperature and gas values from the respective sensors. Based on the gas sensor value,
 * it controls the relay and updates the status of the gas detection. It then sends the collected data as a JSON response.
 **********************************************************************************************************************/
static void getD3D4D5()
{
    // Read temperature and gas value
    readTemp();
    gas_value = analogRead(SENSOR_GAS);

    // Check gas sensor value and update relay, gasStatus
    if (gas_value <= 10) 
    {
        gasStatus = "Normal";
        ringStatus = "OFF";
        digitalWrite(RELAY1, HIGH);
    }
    else
    {
        gasStatus = "Gas detected";
        ringStatus = "ON";
        digitalWrite(RELAY1, LOW);
    }

    // Build JSON and send response
    String s = "{\"D1\": \""+ led1 +"\", \"D2\": \""+ led2 +"\", \"D3\": \""+ ringStatus +"\", \"D4\": \""+ tempValue +"\", \"D5\": \""+ gasStatus +"\"}";
    server.send(200,"application/json",s);
}

/*******************************************************************************************************************//**
 * @brief Turns on LED 1 by calling the turnON_handler for sensor 1 and relay 2.
 * 
 * This function handles turning on LED 1. It calls the `turnON_handler` function to toggle the relay based on the
 * state of sensor 1.
 **********************************************************************************************************************/
static void turnON_LED1()
{
    String led1;
    turnON_handler(SENSOR1, RELAY2, led1, "InsertDB.php");
}

/*******************************************************************************************************************//**
 * @brief Turns off LED 1 by calling the turnOFF_handler for sensor 1 and relay 2.
 * 
 * This function handles turning off LED 1. It calls the `turnOFF_handler` function to toggle the relay based on the
 * state of sensor 1.
 **********************************************************************************************************************/
static void turnOFF_LED1()
{
    String led1;
    turnOFF_handler(SENSOR1, RELAY2, led1, "InsertDB.php");
}

/*******************************************************************************************************************//**
 * @brief Turns on LED 2 by calling the turnON_handler for sensor 2 and relay 3.
 * 
 * This function handles turning on LED 2. It calls the `turnON_handler` function to toggle the relay based on the
 * state of sensor 2.
 **********************************************************************************************************************/
static void turnON_LED2() 
{
    String led2;
    turnON_handler(SENSOR2, RELAY3, led2, "InsertDB2.php");
}

/*******************************************************************************************************************//**
 * @brief Turns off LED 2 by calling the turnOFF_handler for sensor 2 and relay 3.
 * 
 * This function handles turning off LED 2. It calls the `turnOFF_handler` function to toggle the relay based on the
 * state of sensor 2.
 **********************************************************************************************************************/
static void turnOFF_LED2()
{
    String led2;
    turnOFF_handler(SENSOR2, RELAY3, led2, "InsertDB2.php");
}

/*******************************************************************************************************************//**
 * @brief Setup function to initialize hardware, WiFi, and server.
 * 
 * This function is called once when the device starts. It initializes the GPIO pins, connects to WiFi, sets up the
 * mDNS responder, and configures the web server routes.
 **********************************************************************************************************************/
void setup()
{
    // Set pin modes for input sensors and output relays
    pinMode(SENSOR1, INPUT);
    pinMode(SENSOR2, INPUT);
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);

    // Initialize relay states
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, HIGH);
    digitalWrite(RELAY4, HIGH);

    // Initialize led1, led2 values
    led1 = "OFF";
    led2 = "OFF";

    // Start serial communication for debugging
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    WiFi.begin(WiFiName, WiFiPW);

    // Wait for WiFi connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected"); 
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());    // Print ESP8266's IP address

    // Initialize mDNS for local network discovery and set up web routes
    if (mdns.begin("esp8266", WiFi.localIP()))
    {
        Serial.println("MDNS responder started");
        server.on("/", mainPage);
        server.on("/ctrlpage", controlPage);
        server.on("/contact", contactPage);
        server.on("/faq", faqPage);
        server.on("/turnonled1", turnON_LED1);
        server.on("/turnoffled1", turnOFF_LED1);
        server.on("/turnonled2", turnON_LED2);
        server.on("/turnoffled2", turnOFF_LED2);
        server.on("/getstatusD3D4D5", getD3D4D5);
        server.begin();    // Start the web server
    }
}

/*******************************************************************************************************************//**
 * @brief Main loop to handle incoming client requests.
 * 
 * This function is called repeatedly in the main loop. It handles all incoming client requests to the server.
 **********************************************************************************************************************/
void loop()
{
    server.handleClient();    // Handle incoming client requests
}

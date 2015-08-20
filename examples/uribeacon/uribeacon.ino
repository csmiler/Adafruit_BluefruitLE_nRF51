/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*=========================================================================
 The URL that is advertised. It must not longer than 17 bytes
 (excluding http:// and www.) Note: ".com/" ".net/" count as 1
 --------------------------------------------------------------------------*/
 #define URL                             "http://www.adafruit.com"
/*=========================================================================*/


// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

bool isFirmware066orLater;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit UriBeacon Example"));
  Serial.println(F("------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  
  // EddyStone commands are added from firmware 0.6.6
  // fallback to uribeacon command if firmware is 0.6.5 or former
  // Request Bluefruit's firmware version only.
  ble.println(F("ATI=4"));
  ble.readline();
  isFirmware066orLater = (strcmp(ble.buffer, "0.6.6") >= 0);
  ble.waitForOK();
  
  /* Set EddyStone URL beacon data */
  Serial.println(F("Setting uri beacon to Adafruit website: "));

  if (isFirmware066orLater)
  {
    if (! ble.sendCommandCheckOK(F( "AT+EDDYSTONEURL=" URL ))) {
      error(F("Couldnt set, is URL too long !?"));
    }
  }
  else
  {
    // Prompt user to upgrade
    Serial.println(F("There is firmware update available, please upgrade to have more features"));

    // 0.6.5 use AT+BLEURIBEACON command
    if (! ble.sendCommandCheckOK(F( "AT+BLEURIBEACON=" URL ))) {
      error(F("Couldnt set, is URL too long !?"));
    }
  }

  Serial.println(F("**************************************************"));
  Serial.println(F("Please use Google Physical Web application to test"));
  Serial.println(F("**************************************************"));
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  // EddyStone command only available for firmware from 0.6.6
  if (isFirmware066orLater)
  {
    // Print user's option
    Serial.println(F("Please choose:"));
    Serial.println(F("0 : Disable EddyStone URL"));
    Serial.println(F("1 : Enable EddyStone URL"));
    Serial.println(F("2 : Put EddyStone URL to Config Mode"));

    // Get User's input
    char option[BUFSIZE+1];
    getUserInput(option, BUFSIZE);

    // Proccess option
    switch ( option[0] - '0' )
    {
      case 0:
        ble.sendCommandCheckOK(F("AT+EDDYSTONEENABLE=off"));
        break;

      case 1:
        ble.sendCommandCheckOK(F("AT+EDDYSTONEENABLE=on"));
        break;

      case 2:
        Serial.println(F("EddyStone config's mode is enabled for 300 seconds"));
        Serial.println(F("Please use Physical Web app to edit URL"));
        ble.sendCommandCheckOK(F("AT+EDDYSTONECONFIGEN=300"));
        break;

      default:
        Serial.print(F("Invalid input; "));
        Serial.println(option);
        break;
    }
  }
}

/**************************************************************************/
/*!
    @brief  Checks for user input (via the Serial Monitor)
*/
/**************************************************************************/
void getUserInput(char buffer[], uint8_t maxSize)
{
  memset(buffer, 0, maxSize);
  while( Serial.peek() < 0 ) {}
  delay(2);

  uint8_t count=0;

  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && !(Serial.peek() < 0) );
}

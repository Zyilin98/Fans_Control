#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <WiFi.h>
#include <WebServer.h>

extern WebServer server;

void setupServer();
void handleRoot();
void handleChannelDutyCycle();
void handleSpeed();
void handleVoltage();
void handleNaturalWind();
void handleNaturalWindStatus();
void handleDutyCycle();
void handleHeartbeat();
void handleWifiInfo();

#endif

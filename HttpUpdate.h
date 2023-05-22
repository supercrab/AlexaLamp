/**
 *
 * @file HTTPUpdate.h
 * @date 21.06.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the ESP8266 Http Updater.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef HTTPUPDATE_H_
#define HTTPUPDATE_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>

#ifndef HTTPUPDATE_1_2_COMPATIBLE
#define HTTPUPDATE_1_2_COMPATIBLE HTTPCLIENT_1_1_COMPATIBLE
#endif

#ifdef DEBUG_ESP_HTTP_UPDATE
#ifdef DEBUG_ESP_PORT
#define DEBUG_HTTP_UPDATE(fmt, ...) DEBUG_ESP_PORT.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
#endif
#endif

#ifndef DEBUG_HTTP_UPDATE
//#define DEBUG_HTTP_UPDATE(...) do { (void)0; } while(0)
#define DEBUG_HTTP_UPDATE(...) Serial.printf( __VA_ARGS__ )
#endif

/// note we use HTTP client errors too so we start at 100
//TODO - in v3.0.0 make this an enum
constexpr int HTTP_UE_TOO_LESS_SPACE            = (-100);
constexpr int HTTP_UE_SERVER_NOT_REPORT_SIZE    = (-101);
constexpr int HTTP_UE_SERVER_FILE_NOT_FOUND     = (-102);
constexpr int HTTP_UE_SERVER_FORBIDDEN          = (-103);
constexpr int HTTP_UE_SERVER_WRONG_HTTP_CODE    = (-104);
constexpr int HTTP_UE_SERVER_FAULTY_MD5         = (-105);
constexpr int HTTP_UE_BIN_VERIFY_HEADER_FAILED  = (-106);
constexpr int HTTP_UE_BIN_FOR_WRONG_FLASH       = (-107);

enum HTTPUpdateResult {
    HTTP_UPDATE_FAILED,
    HTTP_UPDATE_NO_UPDATES,
    HTTP_UPDATE_OK,
    HTTP_UPDATE_AVAILABLE
};

typedef HTTPUpdateResult t_httpUpdate_return; // backward compatibility

using HTTPUpdateStartCB = std::function<void()>;
using HTTPUpdateEndCB = std::function<void()>;
using HTTPUpdateErrorCB = std::function<void(int)>;
using HTTPUpdateProgressCB = std::function<void(int, int)>;

class HTTPUpdate
{
public:
    HTTPUpdate(void);
    HTTPUpdate(int httpClientTimeout);
    ~HTTPUpdate(void);

    void rebootOnUpdate(bool reboot)
    {
        _rebootOnUpdate = reboot;
    }

    void installUpdate(bool install)
    {
        _installUpdate = install;
    }

    /**
     * set true to follow redirects.
     * @param follow
     * @deprecated Please use `setFollowRedirects(followRedirects_t follow)`
      */
    void followRedirects(bool follow) __attribute__ ((deprecated))
    {
        _followRedirects = follow ? HTTPC_STRICT_FOLLOW_REDIRECTS : HTTPC_DISABLE_FOLLOW_REDIRECTS;
    }
    /**
      * set redirect follow mode. See `followRedirects_t` enum for avaliable modes.
      * @param follow
      */
    void setFollowRedirects(followRedirects_t follow)
    {
        _followRedirects = follow;
    }

    void closeConnectionsOnUpdate(bool sever)
    {
        _closeConnectionsOnUpdate = sever;
    }

    void setLedPin(int ledPin = -1, uint8_t ledOn = HIGH)
    {
        _ledPin = ledPin;
        _ledOn = ledOn;
    }

#if HTTPUPDATE_1_2_COMPATIBLE
    // This function is deprecated, use rebootOnUpdate and the next one instead
    t_httpUpdate_return update(const String& url, const String& currentVersion, const String& httpsFingerprint, bool reboot) __attribute__((deprecated));
    t_httpUpdate_return update(const String& url, const String& currentVersion = "") __attribute__((deprecated));
    t_httpUpdate_return update(const String& url, const String& currentVersion, const String& httpsFingerprint) __attribute__((deprecated));
    t_httpUpdate_return update(const String& url, const String& currentVersion, const uint8_t httpsFingerprint[20]) __attribute__((deprecated)); // BearSSL
#endif
    t_httpUpdate_return update(WiFiClient& client, const String& url, const String& currentVersion = "");

#if HTTPUPDATE_1_2_COMPATIBLE
    // This function is deprecated, use one of the overloads below along with rebootOnUpdate
    t_httpUpdate_return update(const String& host, uint16_t port, const String& uri, const String& currentVersion, bool https, const String& httpsFingerprint, bool reboot) __attribute__((deprecated));

    t_httpUpdate_return update(const String& host, uint16_t port, const String& uri = "/", const String& currentVersion = "") __attribute__((deprecated));
    t_httpUpdate_return update(const String& host, uint16_t port, const String& url, const String& currentVersion, const String& httpsFingerprint) __attribute__((deprecated));
    t_httpUpdate_return update(const String& host, uint16_t port, const String& url, const String& currentVersion, const uint8_t httpsFingerprint[20]) __attribute__((deprecated)); // BearSSL

    t_httpUpdate_return updateNew(const String& deviceId, const String& host, uint16_t port, const String& uri = "/", const String& currentVersion = "") __attribute__((deprecated));
#endif
    t_httpUpdate_return update(WiFiClient& client, const String& host, uint16_t port, const String& uri = "/", const String& currentVersion = "");

#if HTTPUPDATE_1_2_COMPATIBLE
    // This function is deprecated, use rebootOnUpdate and the next one instead
    t_httpUpdate_return updateSpiffs(const String& url, const String& currentVersion, const String& httpsFingerprint, bool reboot) __attribute__((deprecated));
    t_httpUpdate_return updateSpiffs(const String& url, const String& currentVersion = "") __attribute__((deprecated));
    t_httpUpdate_return updateSpiffs(const String& url, const String& currentVersion, const String& httpsFingerprint) __attribute__((deprecated));
    t_httpUpdate_return updateSpiffs(const String& url, const String& currentVersion, const uint8_t httpsFingerprint[20]) __attribute__((deprecated)); // BearSSL
#endif
    t_httpUpdate_return updateFS(WiFiClient& client, const String& url, const String& currentVersion = "");
    t_httpUpdate_return updateSpiffs(WiFiClient& client, const String& url, const String& currentVersion = "") __attribute__((deprecated)) {
        return updateFS(client, url, currentVersion);
    };

    // Notification callbacks
    void onStart(HTTPUpdateStartCB cbOnStart)          { _cbStart = cbOnStart; }
    void onEnd(HTTPUpdateEndCB cbOnEnd)                { _cbEnd = cbOnEnd; }
    void onError(HTTPUpdateErrorCB cbOnError)          { _cbError = cbOnError; }
    void onProgress(HTTPUpdateProgressCB cbOnProgress) { _cbProgress = cbOnProgress; }

    int getLastError(void);
    String getLastErrorString(void);

protected:
    t_httpUpdate_return handleUpdate(HTTPClient& http, const String& currentVersion, bool spiffs = false, const String& deviceId = "");
    bool runUpdate(Stream& in, uint32_t size, const String& md5, int command = U_FLASH);

    // Set the error and potentially use a CB to notify the application
    void _setLastError(int err) {
        _lastError = err;
        if (_cbError) {
            _cbError(err);
        }
    }
    int _lastError;
    bool _rebootOnUpdate = true;
    bool _closeConnectionsOnUpdate = true;
    bool _installUpdate = true;
private:
    int _httpClientTimeout;
    followRedirects_t _followRedirects;

    // Callbacks
    HTTPUpdateStartCB    _cbStart;
    HTTPUpdateEndCB      _cbEnd;
    HTTPUpdateErrorCB    _cbError;
    HTTPUpdateProgressCB _cbProgress;

    int _ledPin;
    uint8_t _ledOn;
};

extern HTTPUpdate ESPhttpUpdate;

#endif /* HTTPUPDATE_H_ */

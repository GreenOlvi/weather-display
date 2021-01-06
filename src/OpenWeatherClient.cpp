#include "OpenWeatherClient.h"
#include "secrets.h"

OpenWeatherClient::OpenWeatherClient(float latitude, float longitude, const char* apiKey)
    : _latitude(latitude), _longitude(longitude), _apiKey(apiKey) {
}

void OpenWeatherClient::update(const unsigned long t) {
    if (WiFi.isConnected() && _weather.updated == 0 && t > _nextRetry) {
        fetchWeather();
        _madeFetchAttempt = true;
    }
}

String OpenWeatherClient::buildUri() {
    String uri = String(_weatherUri);
    uri.replace("{apiKey}", _apiKey);
    uri.replace("{lat}", String(_latitude, 2));
    uri.replace("{lon}", String(_longitude, 2));
    return uri;
}

bool OpenWeatherClient::fetchWeather() {
    //TODO cache response to spifs
    String url = buildUri();

    HTTPClient client;
    client.begin(url);
    _apiQueryCount++;
    _madeFetchAttempt = true;

    int code = client.GET();

#ifdef SERIAL_DEBUG
    Serial.println(code);
#endif

    if (code == 0) {
        _nextRetry = millis() + RetryOnFailDelay;
        return false;
    }

    String payload = client.getString();

#ifdef SERIAL_DEBUG
    // Serial.println(payload);
#endif

    char json[payload.length() + 1];
    payload.toCharArray(json, sizeof(json));
    json[payload.length() + 1] = '\0';

    DynamicJsonDocument doc(PARSING_MEMORY);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
#ifdef SERIAL_DEBUG
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        Serial.println(doc.capacity());
#endif
        _nextRetry = millis() + RetryOnFailDelay;
        return false;
    }

    _weather.updated = millis();
    _weather.timezone = doc["timezone"].as<String>();
    _weather.timezone_offset = doc["timezone_offset"].as<EPOCH>();

    _weather.current.dt = doc["current"]["dt"].as<EPOCH>();
    _weather.current.temp = doc["current"]["temp"].as<float>();
    _weather.current.feels_like = doc["current"]["feels_like"].as<float>();
    _weather.current.icon = doc["current"]["weather"][0]["main"].as<String>();
    _weather.current.description = doc["current"]["weather"][0]["description"].as<String>();
    _weather.current.sunrise = doc["current"]["sunrise"].as<EPOCH>();
    _weather.current.sunset = doc["current"]["sunset"].as<EPOCH>();

    _weather.tomorrow.temp = doc["daily"][1]["temp"]["day"].as<float>();
    _weather.tomorrow.feels_like = doc["daily"][1]["feels_like"]["day"].as<float>();
    _weather.tomorrow.icon = doc["daily"][1]["weather"][0]["main"].as<String>();
    _weather.tomorrow.description = doc["daily"][1]["weather"][0]["description"].as<String>();
    _weather.tomorrow.sunrise = doc["daily"][1]["sunrise"].as<EPOCH>();
    _weather.tomorrow.sunset = doc["daily"][1]["sunset"].as<EPOCH>();
    return true;
}

WeatherData OpenWeatherClient::getCurrentData() {
    return _weather.current;
}

WeatherData OpenWeatherClient::getTomorrowData() {
    return _weather.tomorrow;
}

int OpenWeatherClient::getTimezoneOffset() {
    return _weather.timezone_offset;
}

bool OpenWeatherClient::isUpToDate() {
    return _weather.updated != 0;
}

int OpenWeatherClient::getApiQueryCount() {
    return _apiQueryCount;
}

void OpenWeatherClient::resetApiQueryCount() {
    _apiQueryCount = 0;
}

bool OpenWeatherClient::madeFetchAttempt() {
    return _madeFetchAttempt;
}
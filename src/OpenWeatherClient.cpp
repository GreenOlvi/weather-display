#include "OpenWeatherClient.h"
#include "secrets.h"

OpenWeatherClient::OpenWeatherClient(float latitude, float longitude, const char* apiKey)
    : _latitude(latitude), _longitude(longitude), _apiKey(apiKey) {
}

void OpenWeatherClient::update(const unsigned long t) {
    if (_weather.updated == 0) {
        fetchWeather();
    }
}

String OpenWeatherClient::buildUri() {
    String uri = String(_weatherUri);
    uri.replace("{apiKey}", _apiKey);
    uri.replace("{lat}", String(_latitude, 2));
    uri.replace("{lon}", String(_longitude, 2));
    return uri;
}

void OpenWeatherClient::fetchWeather() {
    String url = buildUri();

    HTTPClient client;
    client.begin(url);

    int code = client.GET();
    if (code == 0) {
        return;
    }

    String payload = client.getString();

    // Serial.println(code);
    // Serial.println(payload);

    char json[payload.length() + 1];
    payload.toCharArray(json, sizeof(json));
    json[payload.length() + 1] = '\0';

    DynamicJsonDocument doc(PARSING_MEMORY);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        Serial.println(doc.capacity());
        return;
    }

    _weather.updated = millis();
    _weather.timezone = doc["timezone"].as<String>();
    _weather.timezone_offset = doc["timezone_offset"].as<EPOCH>();

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


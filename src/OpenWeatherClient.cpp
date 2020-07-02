#include "OpenWeatherClient.h"
#include "secrets.h"

OpenWeatherClient::OpenWeatherClient(int cityId, const char* apiKey) : _cityId(cityId), _apiKey(apiKey) {
}

void OpenWeatherClient::update(const unsigned long t) {
    if (_current.updated == 0) {
        fetchCurrentWeather();
    }
}

String OpenWeatherClient::getCurrentUri() {
    String uri = String(_currentWeatherUri);
    uri.replace("{cityId}", String(_cityId));
    uri.replace("{apiKey}", _apiKey);
    return uri;
}

void OpenWeatherClient::fetchCurrentWeather() {
    String url = getCurrentUri();

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

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    _current.updated = millis();
    _current.temp = doc["main"]["temp"].as<float>();
    _current.feels_like = doc["main"]["feels_like"].as<float>();
    _current.type = doc["weather"][0]["main"].as<String>();
    _current.description = doc["weather"][0]["description"].as<String>();
}

float OpenWeatherClient::getCurrentTemp() {
    return _current.temp;
}

String OpenWeatherClient::getCurrentDescription() {
    return _current.description;
}
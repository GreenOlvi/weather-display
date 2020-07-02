#include "OpenWeatherClient.h"

OpenWeatherClient::OpenWeatherClient(int cityId, const char* apiKey) : _cityId(cityId), _apiKey(apiKey) {
}

void OpenWeatherClient::fetchCurrentWeather() {
}

String OpenWeatherClient::getCurrentUri() {
    String uri = String(_currentWeatherUri);
    uri.replace("{cityId}", String(_cityId));
    uri.replace("{apiKey}", _apiKey);
    return uri;
}

float OpenWeatherClient::getCurrentTemp() {
    String url = getCurrentUri();

    HTTPClient client;
    client.begin(url);

    int code = client.GET();
    Serial.println(code);

    if (code > 0) {
        String payload = client.getString();
        Serial.println(payload);
    }

    return 26.7;
}
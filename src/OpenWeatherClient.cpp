#include "OpenWeatherClient.h"

OpenWeatherClient::OpenWeatherClient(int cityId, const char* apiKey) : _cityId(cityId), _apiKey(apiKey) {
}

void OpenWeatherClient::fetchCurrentWeather() {
}

float OpenWeatherClient::getCurrentTemp() {
    return 26.7;
}
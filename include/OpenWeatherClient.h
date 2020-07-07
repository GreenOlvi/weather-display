#ifndef OPENWEATHERCLIENT_H
#define OPENWEATHERCLIENT_H

#define EPOCH unsigned long

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Updatable.h"

const char _weatherUri[] = "https://api.openweathermap.org/data/2.5/onecall?lat={lat}&lon={lon}&appid={apiKey}&exclude=hourly&units=metric";

struct WeatherData {
    EPOCH dt;
    float temp;
    float feels_like;
    String icon;
    String description;
    EPOCH sunrise;
    EPOCH sunset;
};

struct WeatherResult {
    unsigned long updated;
    String timezone;
    int timezone_offset;
    WeatherData current;
    WeatherData tomorrow;
};

class OpenWeatherClient : Updatable {
    public:
        OpenWeatherClient(float latitude, float longitude, const char* apiKey);
        void update(const unsigned long t) override;
        float getCurrentTemp(void);
        String getCurrentDescription(void);
        float getTomorrowTemp(void);
        String getTomorrowDescription(void);

    private:
        float _latitude, _longitude;
        const char* _apiKey;

        String buildUri(void);
        void fetchWeather(void);
        WeatherResult _weather;
};

#endif
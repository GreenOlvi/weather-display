#ifndef OPENWEATHERCLIENT_H
#define OPENWEATHERCLIENT_H

#define EPOCH unsigned long
#define PARSING_MEMORY 7500

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Updatable.h"

const char _weatherUri[] = "https://api.openweathermap.org/data/2.5/onecall?lat={lat}&lon={lon}&appid={apiKey}&exclude=hourly&units=metric";
#define RetryOnFailDelay 60000

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
        WeatherData getCurrentData(void);
        WeatherData getTomorrowData(void);
        int getTimezoneOffset(void);
        bool isUpToDate();
        int getApiQueryCount();
        void resetApiQueryCount();
        bool madeFetchAttempt();

    private:
        float _latitude, _longitude;
        const char* _apiKey;
        int _apiQueryCount = 0;
        EPOCH _nextRetry = 0;
        bool _madeFetchAttempt = false;

        String buildUri(void);
        bool fetchWeather(void);
        WeatherResult _weather;
};

#endif
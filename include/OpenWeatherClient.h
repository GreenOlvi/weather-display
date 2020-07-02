#ifndef OPENWEATHERCLIENT_H
#define OPENWEATHERCLIENT_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Updatable.h"

const char _currentWeatherUri[] = "https://api.openweathermap.org/data/2.5/weather?id={cityId}&appid={apiKey}&units=metric";

struct CurrentWeather {
    unsigned long updated;
    float temp;
    float feels_like;
    String type;
    String description;
};

class OpenWeatherClient : Updatable {
    public:
        OpenWeatherClient(int cityId, const char* apiKey);
        void update(const unsigned long t) override;
        float getCurrentTemp(void);
        String getCurrentDescription(void);

    private:
        int _cityId;
        const char* _apiKey;

        String getCurrentUri(void);
        void fetchCurrentWeather(void);
        CurrentWeather _current;
};

#endif
#ifndef OPENWEATHERCLIENT_H
#define OPENWEATHERCLIENT_H

#include <HTTPClient.h>

const char _currentWeatherUri[] = "https://api.openweathermap.org/data/2.5/weather?id={cityId}&appid={apiKey}&units=metric";

class OpenWeatherClient {
    public:
        OpenWeatherClient(int cityId, const char* apiKey);
        void fetchCurrentWeather(void);
        float getCurrentTemp(void);

    private:
        int _cityId;
        const char* _apiKey;
        String getCurrentUri();
};

#endif
#ifndef OPENWEATHERCLIENT_H
#define OPENWEATHERCLIENT_H

class OpenWeatherClient {
    public:
        OpenWeatherClient(int cityId, const char* apiKey);
        void fetchCurrentWeather(void);
        float getCurrentTemp(void);

    private:
        int _cityId;
        const char* _apiKey;
};

#endif
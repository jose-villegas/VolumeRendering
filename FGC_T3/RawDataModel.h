#pragma once
#include <string>

class RawDataModel
{
    public:
        std::string sModelName;
        bool isLoaded;
        int width;
        int height;
        int numCuts;

        void Load(std::string sFilePath, int width, int height, int numCuts);
        RawDataModel(void);
        ~RawDataModel(void);
};


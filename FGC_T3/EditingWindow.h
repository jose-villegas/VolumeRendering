#pragma once
#include "commons.h"
#include "RawDataModel.h"
#include "UIBuilder.h"

class EditingWindow
{
    private:
        RawDataModel * rawModel;
        std::thread * windowThread;
        UIBuilder gui;
        std::array<float, 256> histogram;
        sf::RectangleShape line[256];
        bool isHistLoaded;

        static void _windowRender(EditingWindow * eWin);
    public:
        sf::RenderWindow * parent;
        sf::RenderWindow * window;

        void init(sf::RenderWindow * parent);
        void loadHistogram(RawDataModel * rawModel);
        EditingWindow(void);
        ~EditingWindow(void);
};


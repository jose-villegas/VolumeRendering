#pragma once
#include "commons.h"
#include "RawDataModel.h"

class EditingWindow
{
    private:
        RawDataModel * rawModel;
        std::thread * windowThread;
        std::array<int, 256> histogram;

        static void _windowRender(EditingWindow * eWin);
    public:
        sf::RenderWindow * parent;
        sf::RenderWindow * window;

        void init(sf::RenderWindow * parent);
        void loadHistogram(RawDataModel * rawModel);
        EditingWindow(void);
        ~EditingWindow(void);
};


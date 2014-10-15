#pragma once
#include "commons.h"
#include "RawDataModel.h"
#include "UIBuilder.h"
#include "TransferFunction.h"
#define DRAG_TOLERANCE 5

class EditingWindow
{
    private:
        RawDataModel * rawModel;
        std::thread * windowThread;
        std::array<float, 256> histogram;
        bool isHistLoaded;

        static void _windowRender(EditingWindow * eWin);

        static void updateTransferFunction(EditingWindow * eWin);

        static bool isMouseOver(EditingWindow * eWin, sf::CircleShape &circle);

    public:
        sf::RenderWindow * parent;
        sf::RenderWindow * window;

        void init(sf::RenderWindow * parent);
        void loadHistogram(RawDataModel * rawModel);
        EditingWindow(void);
        ~EditingWindow(void);
};


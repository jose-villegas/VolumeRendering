#pragma once
#include "commons.h"
#include "RawDataModel.h"
#include "UIBuilder.h"
#include "TransferFunction.h"
#define DRAG_TOLERANCE 5

class EditingWindow {
    private:
        RawDataModel * rawModel;
        std::array<float, 256> histogram;
        bool isHistLoaded;
        bool dragStarted;
        int mouseOverIndex;

        static bool _isMouseOver(EditingWindow * eWin, sf::CircleShape &circle);
        static void _drawControlPointCircles(int i, sf::CircleShape &circle, EditingWindow * eWin);
        static void _drawHistogram(sf::RectangleShape &line, EditingWindow * eWin, sf::RectangleShape &indicator);
        static void _drawTransferFuncPlot(int i, sf::CircleShape &circle, EditingWindow * eWin);
        static void _initEditingWindowUI(UIBuilder &ui, EditingWindow * eWin);
        static void _initRenderContext(EditingWindow * eWin);
        static void _updateTransferFunction(EditingWindow * eWin);
        static void _windowRender(EditingWindow * eWin);
        static void _inputControl(EditingWindow * eWin, UIBuilder &ui);
    public:
        std::thread * windowThread;
        sf::RenderWindow * parent;
        sf::RenderWindow * window;

        void init(sf::RenderWindow * parent);
        void loadHistogram(RawDataModel * rawModel);
        EditingWindow(void);
        ~EditingWindow(void);
};


#pragma once
#include "commons.h"

typedef std::map<std::string, TwBar *> UIBars;

class UIBuilder
{
    private:
        sf::WindowHandle hwnd;
        UIBars _uiBars;
    public:
        void init(int width, int height);
        void setHwnd(sf::WindowHandle val) { hwnd = val; }
        void addBar(std::string sBarName);
        void addBar(std::string sBarName, int iPosition[2], int iSize[2]);
        void setBarPosition(std::string sBarName, int x, int y);
        void setBarSize(std::string sBarName, int w, int h);
        void addVariable(std::string sBarName, std::string sName, TwType varType, void * var, std::string sVarParams);
        void addCheckbox(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addIntegerNumber(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addFloatNumber(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addDoubleNumber(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addColorControls(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addRotationControls(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addDirectionControls(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addTextfield(std::string sBarName, std::string sName, void * var, std::string sVarParams);
        void addLabel(std::string sBarName, std::string sLabelText);
        void addButton(std::string sBarName, std::string sLabelText, TwButtonCallback bCallBackFunc, void * clientData, std::string sParams);
        void addFileDialogButton(std::string sBarName, std::string sLabelText, char * destString, std::string sParams);
        UIBuilder(void);
        ~UIBuilder(void);
};

void TW_CALL fileDialogButtonOnCB(void * clientData);
#include "UIBuilder.h"

UIBuilder::UIBuilder(void)
{
}


UIBuilder::~UIBuilder(void)
{
}

void UIBuilder::init(int width, int height)
{
    TwInit(TW_OPENGL, NULL);
    // Tell the window size to AntTweakBar
    TwWindowSize(width , height);
}

void UIBuilder::addBar(std::string sBarName)
{
    TwBar * bar = TwNewBar(sBarName.c_str());
    _uiBars[sBarName] = bar;
}

void UIBuilder::addBar(std::string sBarName, int iPosition[2], int iSize[2])
{
    TwBar * bar = TwNewBar(sBarName.c_str());

    if (iPosition != NULL) { TwSetParam(bar, NULL, "position", TW_PARAM_INT32, 2, iPosition); }

    if (iSize != NULL) { TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, iSize); }

    _uiBars[sBarName] = bar;
}

void UIBuilder::setBarPosition(std::string sBarName, int x, int y)
{
    TwBar * bar = _uiBars[sBarName];
    int barPosition[2] = {x, y};
    TwSetParam(bar, NULL, "position", TW_PARAM_INT32, 2, barPosition);
}

void UIBuilder::setBarSize(std::string sBarName, int w, int h)
{
    TwBar * bar = _uiBars[sBarName];
    int barSize[2] = {w, h};
    TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
}

void UIBuilder::addVariable(std::string sBarName, std::string sName, TwType varType, void * var, std::string sVarParams)
{
    TwBar * bar = _uiBars[sBarName];
    TwAddVarRW(bar, sName.c_str(), varType, var, sVarParams.c_str());
}

void UIBuilder::addCheckbox(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_BOOLCPP, var, sVarParams);
}

void UIBuilder::addIntegerNumber(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_INT32, var, sVarParams);
}

void UIBuilder::addFloatNumber(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_FLOAT, var, sVarParams);
}

void UIBuilder::addDoubleNumber(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_DOUBLE, var, sVarParams);
}

void UIBuilder::addColorControls(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_COLOR4F, var, sVarParams);
}

void UIBuilder::addRotationControls(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_QUAT4F, var, sVarParams);
}

void UIBuilder::addDirectionControls(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_DIR3F, var, sVarParams);
}

void UIBuilder::addTextfield(std::string sBarName, std::string sName, void * var, std::string sVarParams)
{
    addVariable(sBarName, sName, TW_TYPE_CDSTRING, var, sVarParams);
}

void UIBuilder::addLabel(std::string sBarName, std::string sLabelText)
{
    TwBar * bar = _uiBars[sBarName];
    TwAddButton(bar, sLabelText.c_str(), NULL, NULL, "");
}

void UIBuilder::addButton(std::string sBarName, std::string sLabelText, TwButtonCallback bCallBackFunc, void * clientData, std::string sParams)
{
    TwBar * bar = _uiBars[sBarName];
    TwAddButton(bar, sLabelText.c_str(), bCallBackFunc, clientData, sParams.c_str());
}

void UIBuilder::addFileDialogButton(std::string sBarName, std::string sLabelText, char * destString, std::string sParams)
{
    TwBar * bar = _uiBars[sBarName];
    TwAddButton(bar, sLabelText.c_str(), fileDialogButtonOnCB, destString, sParams.c_str());
}

void TW_CALL fileDialogButtonOnCB(void * clientData)
{
    struct OpenFileDialog   // struct's as good as class
    {
        static void show(char * sFilePath)
        {
            OPENFILENAME ofn;       // common dialog box structure
            char szFile[1024] = {""};       // buffer for file name
            HWND hwnd = nullptr;              // owner window
            HANDLE hf;              // file handle
            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
            // use the contents of szFile to initialize itself.
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "RAW\0*.raw\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            // Display the Open dialog box.
            if (GetOpenFileName(&ofn) == TRUE)
            {
                std::string filename = szFile;
                const size_t last_slash_idx = filename.find_last_of("\\/");
                memcpy(sFilePath, &szFile[last_slash_idx + 1], 1024);
            }
        }
    };
    std::thread dialogThread(OpenFileDialog::show, ((char *)clientData));
    dialogThread.join();
}



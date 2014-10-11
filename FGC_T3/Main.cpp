#include "commons.h"
#include "UIBuilder.h"
#include "RawDataModel.h"
#include "MainData.h"
#include "Camera.h"
#include "EditingWindow.h"
#define WINDOW_OFFSET 200

// OpenGL Setup Before Rendering to Context
void openglSetup(sf::VideoMode desktop);
// SFML Context Settings for OpenGL Rendering
sf::ContextSettings openglWindowContext();
// GLEW Initializer
void initGlew();
// Setup AntTweakBar
void guiSetup(sf::Window &window, UIBuilder &gui);
// Main Render-Logic Loop
void Render(sf::RenderWindow &window, sf::Clock &frameClock);
// Active Volume Data
RawDataModel * rawModel;
EditingWindow * eWindow;

int main()
{
    UIBuilder gui;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    desktop.height = desktop.height - WINDOW_OFFSET;
    desktop.width = desktop.width - WINDOW_OFFSET;
    sf::RenderWindow window(desktop, "Volume Rendering!", sf::Style::Default, openglWindowContext());
    gui.setHwnd(window.getSystemHandle()) ;
    window.setActive(true);
    // Initialize GLEW
    initGlew();
    // Setup MainEngine to hold important shader data
    rawModel = new RawDataModel();
    MainData::rootWindow = &window;
    // Setup for GUI
    eWindow = new EditingWindow();
    guiSetup(window, gui);
    eWindow->init(&window);
    TransferFunction::addControlPoint(255 * 0.9f, 255 * 0.7f, 255 * 0.6f, 0);
    TransferFunction::addControlPoint(255 * 0.9f, 255 * 0.7f, 255 * 0.6f, 80);
    TransferFunction::addControlPoint(255 * 1.0f, 255 * 1.0f, 255 * 0.8f, 82);
    TransferFunction::addControlPoint(255 * 1.0f, 255 * 1.0f, 255 * 0.8f, 255);
    TransferFunction::addControlPoint(255 * 0.0f, 80);
    TransferFunction::addControlPoint(255 * 0.05f, 63);
    TransferFunction::addControlPoint(255 * 0.0f, 40);
    TransferFunction::addControlPoint(255 * 1.0f, 255);
    TransferFunction::addControlPoint(255 * 0.0f, 0);
    TransferFunction::addControlPoint(255 * 0.9f, 82);
    TransferFunction::addControlPoint(255 * 0.2f, 60);
    TransferFunction::getSmoothFunction(rawModel->transferFunc);
    rawModel->updateTransferFunc1DTex();
    // Setup OpenGL to Current Context
    openglSetup(desktop);
    // Initialze Main Loop
    Render(window, *MainData::frameClock);
    // Free Memory
    //delete rawModel;
    //delete eWindow;
    return 0;
}

sf::ContextSettings openglWindowContext()
{
    sf::ContextSettings settings;
    settings.depthBits = 8;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 0;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    return settings;
}

void openglSetup(sf::VideoMode desktop)
{
}

void initGlew()
{
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile

    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }
}

struct Callbacks
{
    static void TW_CALL loadModelClick(void * clientData)
    {
        rawModel->load(rawModel->sModelName, rawModel->width, rawModel->height, rawModel->numCuts);

        if (rawModel->isLoaded) { eWindow->loadHistogram(rawModel); }
    }
};

void guiSetup(sf::Window &window, UIBuilder &gui)
{
    gui.init(window.getSize().x, window.getSize().y);
    // Custom Point Type
    struct Point { float X, Y, Z; };
    TwStructMember pointMembers[] =
    {
        { "X", TW_TYPE_FLOAT, offsetof(Point, X), " Step=0.001 keyIncr='d' keyDecr='a' "},
        { "Y", TW_TYPE_FLOAT, offsetof(Point, Y), " Step=0.001 keyIncr='s' keyDecr='w' " },
        { "Z", TW_TYPE_FLOAT, offsetof(Point, Z), " Step=0.001 keyIncr='-' keyDecr='+' " }
    };
    TwType pointType = TwDefineStruct("POINT", pointMembers, 3, sizeof(Point), NULL, NULL);
    // Model Loading
    gui.addBar("Archivo");
    gui.setBarPosition("Archivo", 5, 5);
    gui.setBarSize("Archivo", 200, 130);
    gui.addFileDialogButton("Archivo", "Seleccionar .RAW", rawModel->sModelName, "");
    gui.addTextfield("Archivo", "Modelo: ", &rawModel->sModelName, "");
    gui.addIntegerNumber("Archivo", "Ancho", &rawModel->width, "");
    gui.addIntegerNumber("Archivo", "Largo", &rawModel->height, "");
    gui.addIntegerNumber("Archivo", "Cortes", &rawModel->numCuts, "");
    gui.addButton("Archivo", "Cargar Modelo Seleccionado", Callbacks::loadModelClick, NULL, "");
    // Camera Controls
    gui.addBar("Camara");
    gui.setBarPosition("Camara", 5, 140);
    gui.setBarSize("Camara", 200, 180);
    gui.addVariable("Camara", "Posicion", pointType, &Camera::position[0], "opened=true");
    gui.addDirectionControls("Camara", "Direccion", &Camera::direction[0], "opened=true");
    // Model Controls
    gui.addBar("Modelo");
    gui.setBarPosition("Modelo", 5, 325);
    gui.setBarSize("Modelo", 200, 120);
    gui.addRotationControls("Modelo", "Rotacion", &rawModel->rotation, "opened=true showval=true");
}

void Render(sf::RenderWindow &window, sf::Clock &frameClock)
{
    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            // Send event to AntTweakBar
            int handled = TwEventSFML(&event, 1, 6);

            if (!handled)
            {
                if (event.type == sf::Event::Closed)
                {
                    rawModel->isLoaded = false;
                    window.close();
                }
                else if (event.type == sf::Event::Resized)
                {
                    // adjust the viewport when the window is resized
                    glViewport(0, 0, event.size.width, event.size.height);
                }
            }
        }

        window.clear();
        // Render OpenGL
        rawModel->render();
        // Draw UI
        TwDraw();
        // End Frame
        window.display();
        frameClock.restart();
    }
}
#include "commons.h"
#include "UIBuilder.h"
#include "RawDataModel.h"
#include "MainData.h"
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
void Render(sf::Window &window, sf::Clock &clock);
// Active Volume Data
RawDataModel * rawModel;

int main()
{
    UIBuilder gui;
    sf::Clock clock;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    desktop.height = desktop.height - WINDOW_OFFSET;
    desktop.width = desktop.width - WINDOW_OFFSET;
    sf::Window window(desktop, "SFML works!", sf::Style::Default, openglWindowContext());
    gui.setHwnd(window.getSystemHandle()) ;
    window.setActive(true);
    // Initialize GLEW
    initGlew();
    // Setup MainEngine to hold important shader data
    rawModel = new RawDataModel();
    MainData::rootWindow = &window;
    MainData::mainClock = &clock;
    // Setup AntTweakBar for GUI
    guiSetup(window, gui);
    // Setup OpenGL to Current Context
    openglSetup(desktop);
    // Initialze Main Loop
    Render(window, clock);
    return 0;
}

sf::ContextSettings openglWindowContext()
{
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 0;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    return settings;
}

void openglSetup(sf::VideoMode desktop)
{
    // Set color and depth clear value
    glClearDepth(1.f);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    // Setup a perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.f, desktop.width / desktop.height, 1.f, 500.f);
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
    }
};

void guiSetup(sf::Window &window, UIBuilder &gui)
{
    gui.init(window.getSize().x, window.getSize().y);
    gui.addBar("Archivo");
    gui.setBarPosition("Archivo", 5, 5);
    gui.addFileDialogButton("Archivo", "Seleccionar .RAW", rawModel->sModelName, "");
    gui.addTextfield("Archivo", "Modelo: ", &rawModel->sModelName, "");
    gui.addIntegerNumber("Archivo", "Ancho", &rawModel->width, "");
    gui.addIntegerNumber("Archivo", "Largo", &rawModel->height, "");
    gui.addIntegerNumber("Archivo", "Cortes", &rawModel->numCuts, "");
    gui.addButton("Archivo", "Cargar Modelo Seleccionado", Callbacks::loadModelClick, NULL, "");
}

void Render(sf::Window &window, sf::Clock &clock)
{
    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            // Send event to AntTweakBar
            int handled = TwEventSFML(&event, 1, 6); // Assume SFML version 1.6 here

            if (event.type == sf::Event::Closed)
            {
                // TODO Clear Memory
                rawModel->isLoaded = false;
                window.close();
            }
            else if (event.type == sf::Event::Resized)
            {
                // adjust the viewport when the window is resized
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Render OpenGL
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //glRotatef(clock.getElapsedTime().asSeconds() * 50, 1.f, 0.f, 0.f);
        //glRotatef(clock.getElapsedTime().asSeconds() * 30, 0.f, 1.f, 0.f);
        //glRotatef(clock.getElapsedTime().asSeconds() * 90, 0.f, 0.f, 1.f);
        rawModel->render();
        // Draw UI
        TwDraw();
        // End Frame
        window.display();
    }
}



#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>

#define WINDOW_OFFSET 200

// OpenGL Setup Before Rendering to Context
void OpenGL_Setup();
// SFML Context Settings for OpenGL Rendering
sf::ContextSettings OpenGL_WindowContextSetup();
// GLEW Initializer
void InitGLEW();

int main()
{
    sf::Clock clock;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    desktop.height = desktop.height - WINDOW_OFFSET;
    desktop.width = desktop.width - WINDOW_OFFSET;
    sf::RenderWindow window(desktop, "SFML works!", sf::Style::Default, OpenGL_WindowContextSetup());
    window.setActive(true);
    // Setup OpenGL to Current Context
    OpenGL_Setup();
    // Initialize GLEW
    InitGLEW();
    bool running = true;

    while (running)
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                running = false;
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
        glTranslatef(0.f, 0.f, -7.f);
        glRotatef(clock.getElapsedTime().asSeconds() * 50, 1.f, 0.f, 0.f);
        glRotatef(clock.getElapsedTime().asSeconds() * 30, 0.f, 1.f, 0.f);
        glRotatef(clock.getElapsedTime().asSeconds() * 90, 0.f, 0.f, 1.f);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);	// Color Blue
        glVertex3f(1.0f, 1.0f, -1.0f);	// Top Right Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f, -1.0f);	// Top Left Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f, 1.0f);	// Bottom Left Of The Quad (Top)
        glVertex3f(1.0f, 1.0f, 1.0f);	// Bottom Right Of The Quad (Top)
        glColor3f(1.0f, 0.5f, 0.0f);	// Color Orange
        glVertex3f(1.0f, -1.0f, 1.0f);	// Top Right Of The Quad (Bottom)
        glVertex3f(-1.0f, -1.0f, 1.0f);	// Top Left Of The Quad (Bottom)
        glVertex3f(-1.0f, -1.0f, -1.0f);	// Bottom Left Of The Quad (Bottom)
        glVertex3f(1.0f, -1.0f, -1.0f);	// Bottom Right Of The Quad (Bottom)
        glColor3f(1.0f, 0.0f, 0.0f);	// Color Red
        glVertex3f(1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Front)
        glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Front)
        glVertex3f(-1.0f, -1.0f, 1.0f);	// Bottom Left Of The Quad (Front)
        glVertex3f(1.0f, -1.0f, 1.0f);	// Bottom Right Of The Quad (Front)
        glColor3f(1.0f, 1.0f, 0.0f);	// Color Yellow
        glVertex3f(1.0f, -1.0f, -1.0f);	// Top Right Of The Quad (Back)
        glVertex3f(-1.0f, -1.0f, -1.0f);	// Top Left Of The Quad (Back)
        glVertex3f(-1.0f, 1.0f, -1.0f);	// Bottom Left Of The Quad (Back)
        glVertex3f(1.0f, 1.0f, -1.0f);	// Bottom Right Of The Quad (Back)
        glColor3f(0.0f, 0.0f, 1.0f);	// Color Blue
        glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Left)
        glVertex3f(-1.0f, 1.0f, -1.0f);	// Top Left Of The Quad (Left)
        glVertex3f(-1.0f, -1.0f, -1.0f);	// Bottom Left Of The Quad (Left)
        glVertex3f(-1.0f, -1.0f, 1.0f);	// Bottom Right Of The Quad (Left)
        glColor3f(1.0f, 0.0f, 1.0f);	// Color Violet
        glVertex3f(1.0f, 1.0f, -1.0f);	// Top Right Of The Quad (Right)
        glVertex3f(1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Right)
        glVertex3f(1.0f, -1.0f, 1.0f);	// Bottom Left Of The Quad (Right)
        glVertex3f(1.0f, -1.0f, -1.0f);	// Bottom Right Of The Quad (Right)
        glEnd();
        // End Frame
        window.display();
    }

    return 0;
}

sf::ContextSettings OpenGL_WindowContextSetup()
{
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    return settings;
}

void OpenGL_Setup()
{
    // Set color and depth clear value
    glClearDepth(1.f);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    // Setup a perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.f, 1.f, 1.f, 500.f);
}

void InitGLEW()
{
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile

    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
    }
}

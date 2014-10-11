#include "EditingWindow.h"

EditingWindow::EditingWindow(void)
{
    histogram.fill(0);
    rawModel = NULL;
    windowThread = NULL;
    parent = window = NULL;
    isHistLoaded = false;

    for (int i = 0; i < 256; i++)
    {
        line[i].setFillColor(sf::Color::Red);
        line[i].setPosition(5 + i * 3, 260);
        line[i].rotate(270);
    }
}


EditingWindow::~EditingWindow(void)
{
}

void EditingWindow::init(sf::RenderWindow * parent)
{
    this->parent = parent;
    windowThread = new std::thread(&EditingWindow::_windowRender, this);
}

void EditingWindow::_windowRender(EditingWindow * eWin)
{
    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 8;
    settings.majorVersion = 4;
    settings.minorVersion = 0;
    eWin->window = new sf::RenderWindow(sf::VideoMode(775, 285, 32), "Funcion de Transferencia", sf::Style::Titlebar, settings);
    eWin->window->setPosition(sf::Vector2i(0, 0));
    bool dragStarted = false;
    sf::RectangleShape indicator[256];
    sf::CircleShape circle(4);
    circle.setOutlineThickness(1);
    UIBuilder ui;
    ui.addBar("Point");
    ui.setBarSize("Point", 200, 200);
    ui.setBarPosition("Point", eWin->parent->getSize().x - 205, 5);

    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++)
    {
        ui.addColorControls("Point", "Point " + std::to_string(i), TransferFunction::getControlPointColors(i), "opened=true");
    }

    for (int i = 0; i < 256; i++)
    {
        indicator[i] = sf::RectangleShape(sf::Vector2f(2, 10));
        indicator[i].setFillColor(sf::Color(i, i, i, i));
        indicator[i].setPosition(5 + i * 3, 260);
    }

    // Histogram
    sf::Color outline;

    while (eWin->parent->isOpen() && eWin->window->isOpen())
    {
        sf::Event event;

        if (eWin->isHistLoaded)
        {
            while (eWin->window->pollEvent(event))
            {
                outline = sf::Color::Cyan;

                if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Middle))
                {
                    updateTransferFunction(eWin);
                    TwRemoveAllVars(ui.getBar("Point"));

                    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++)
                    {
                        ui.addColorControls("Point", "Point " + std::to_string(i), TransferFunction::getControlPointColors(i), "opened=true");
                    }
                }
            }

            eWin->window->clear(sf::Color::Color(20, 20, 20, 255));

            for (int i = 0; i < 256; i++)
            {
                eWin->line[i].setSize(sf::Vector2f(log10(eWin->histogram[i] * 9 + 1) * 256.0f, 2));
                eWin->window->draw(eWin->line[i]);
                //////////////////////////////////////////////////////////////////////////
                GLubyte * color = eWin->rawModel->transferFunc[i];
                indicator[i].setPosition(5 + i * 3, 273);
                indicator[i].setFillColor(sf::Color((int) * (color), (int) * (color + 1), (int) * (color + 2), (int) * (color + 3)));
                eWin->window->draw(indicator[i]);
                indicator[i].setPosition(5 + i * 3, 260);
                indicator[i].setFillColor(sf::Color(i, i, i, i));
                eWin->window->draw(indicator[i]);
            }

            for (int i = 0; i < TransferFunction::getControlPoints().size(); i++)
            {
                // Circles for Controls Points
                sf::Color rgba;
                rgba.r = TransferFunction::getControlPoints()[i].rgba[0] * 255;
                rgba.g = TransferFunction::getControlPoints()[i].rgba[1] * 255;
                rgba.b = TransferFunction::getControlPoints()[i].rgba[2] * 255;
                rgba.a = TransferFunction::getControlPoints()[i].rgba[3] * 255;
                circle.setOutlineThickness(1);
                circle.setFillColor(rgba);
                circle.setOutlineColor(sf::Color::Cyan);
                circle.setPosition(TransferFunction::getControlPoints()[i].isoValue * 3 - 3,
                                   255 - TransferFunction::getControlPoints()[i].rgba[3] * 255);

                if (isMouseOver(eWin, circle))
                {
                    circle.setOutlineColor(sf::Color::Green);

                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    {
                        if (!dragStarted)
                        {
                            dragStarted = true;
                        }
                        else
                        {
                            circle.setOutlineThickness(2);
                            int finalIsoValue = sf::Mouse::getPosition(*eWin->window).x / 3;
                            int finalAlphaValue = 255 - sf::Mouse::getPosition(*eWin->window).y + 3;
                            TransferFunction::deleteAlphaControlPoint(i);
                            finalIsoValue = i == 0 ? 0 : i == TransferFunction::getControlPoints().size() ? 255 : finalIsoValue;
                            TransferFunction::addControlPoint(rgba.r, rgba.g, rgba.b, finalAlphaValue, finalIsoValue);
                        }
                    }
                    else
                    {
                        dragStarted = false;
                    }
                }

                // Plotting lines
                if (i < TransferFunction::getControlPoints().size() - 1)
                {
                    sf::RectangleShape plotLine;
                    sf::Vector2f nextPos = sf::Vector2f(TransferFunction::getControlPoints().at(i + 1).isoValue * 3 - 3 + 4,
                                                        255 - TransferFunction::getControlPoints().at(i + 1).rgba[3] * 255 + 4);
                    sf::Vector2f currentPos = sf::Vector2f(circle.getPosition().x + 4, circle.getPosition().y + 4);
                    float xDiff = currentPos.x - nextPos.x;
                    float yDiff = currentPos.y - nextPos.y;
                    float angle = atan2(yDiff, xDiff) * (180 / 3.14);
                    float distSize = sqrt(pow(xDiff, 2) + pow(yDiff, 2));
                    plotLine.rotate(90 + angle);
                    plotLine.setPosition(currentPos);
                    plotLine.setSize(sf::Vector2f(1, distSize));
                    eWin->window->draw(plotLine);
                }

                eWin->window->draw(circle);
            }

            // Update Transfer Function Real Time
            TransferFunction::getLinearFunction(eWin->rawModel->transferFunc);
            eWin->rawModel->updateTransferFunc1DTex();
        }

        eWin->window->display();
    }

    return;
}

void EditingWindow::loadHistogram(RawDataModel * rawModel)
{
    this->rawModel = rawModel;
    isHistLoaded = false;
    histogram.fill(0);
    std::vector<std::thread> worker(rawModel->numCuts);
    std::vector<std::array<float, 256>> results(rawModel->numCuts);
    struct Work
    {
        static void cutHistogram(GLubyte * cut, int size, std::array<float, 256> * result)
        {
            for (int i = 0; i < size; i++)
            {
                result->at((int)(*(cut + i))) += 1;
            }

            return;
        }

        static std::array<float, 256> sumArrays(std::array<float, 256> a, std::array<float, 256> b)
        {
            std::array<float, 256> result;
            std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::plus<float>());
            return result;
        }
    };
    int sizeOfCut = rawModel->width * rawModel->height;

    for (int i = 0; i < rawModel->numCuts; i++)
    {
        worker[i] = std::thread(Work::cutHistogram, rawModel->data + sizeOfCut * i, sizeOfCut, &results[i]);
    }

    for (int i = 0; i < rawModel->numCuts; i++)
    {
        worker[i].join();
    }

    histogram = std::accumulate(results.begin(), results.end(), histogram, Work::sumArrays);
    float maxHistValue = *std::max_element(histogram.begin(), histogram.end());
    std::transform(histogram.begin(), histogram.end(), histogram.begin(), std::bind1st(std::multiplies<float>(), 1 / maxHistValue));
    isHistLoaded = true;
}
bool EditingWindow::isMouseOver(EditingWindow * eWin, sf::CircleShape &circle)
{
    sf::Vector2f mousePos(sf::Mouse::getPosition(*eWin->window));

    if
    (
        mousePos.x > circle.getPosition().x - 5 &&
        mousePos.x <= circle.getPosition().x + circle.getGlobalBounds().width + 5 &&
        mousePos.y > circle.getPosition().y - 5 &&
        mousePos.y <= circle.getPosition().y + circle.getGlobalBounds().height + 5
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void EditingWindow::updateTransferFunction(EditingWindow * eWin)
{
    if (sf::Mouse::getPosition(*eWin->window).y > 2 && sf::Mouse::getPosition(*eWin->window).x > 0 &&
            sf::Mouse::getPosition(*eWin->window).x < 769 && sf::Mouse::getPosition(*eWin->window).y < 259)
    {
        //clickingPoints.push_back(sf::CircleShape(2));
        //clickingPoints.back().setFillColor(sf::Color::Transparent);
        //clickingPoints.back().setOutlineThickness(1);
        //clickingPoints.back().setPosition((sf::Vector2f)sf::Mouse::getPosition(*eWin->window));
        TransferFunction::addControlPoint(255, 255, 255, 255 - sf::Mouse::getPosition(*eWin->window).y + 3,
                                          sf::Mouse::getPosition(*eWin->window).x / 3);
    }
}

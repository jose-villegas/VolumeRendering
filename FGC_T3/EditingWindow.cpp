#include "EditingWindow.h"

EditingWindow::EditingWindow(void)
{
    histogram.fill(0);
    dragStarted = false;
    isHistLoaded = false;
    mouseOverIndex = -1;
    rawModel = NULL;
    windowThread = NULL;
    parent = window = NULL;
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
    // Drawing Figures
    sf::RectangleShape indicator;
    sf::RectangleShape line;
    sf::CircleShape circle(4);
    sf::Event event;
    // AntTweakBar Handling
    UIBuilder ui;
    // Dragging Control Variables
    _initRenderContext(eWin);
    _initEditingWindowUI(ui, eWin);
    // Setup Figures
    circle.setOutlineThickness(1);
    line.setFillColor(sf::Color::Red);
    line.rotate(270);
    // Histogram Drawing and Control

    while (eWin->parent->isOpen() && eWin->window->isOpen()) {
        if (eWin->isHistLoaded) {
            while (eWin->window->pollEvent(event)) {
                if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                    _updateTransferFunction(eWin);
                    TwRemoveAllVars(ui.getBar("Points"));

                    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
                        ui.addColorControls("Points", "Point " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
                    }
                }
            }

            eWin->window->clear(sf::Color::Color(20, 20, 20, 255));
            _drawHistogram(line, eWin, indicator);

            for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
                _drawControlPointCircles(i, circle, eWin);
                _drawTransferFuncPlot(i, circle, eWin);
            }

            // Update Transfer Function Real-Time
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
    // Local Struct Functions just to avoid clogging the main class
    struct Work {
        static void cutHistogram(GLubyte * cut, int size, std::array<float, 256> * result)
        {
            for (int i = 0; i < size; i++) {
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

    for (int i = 0; i < rawModel->numCuts; i++) {
        worker[i] = std::thread(Work::cutHistogram, rawModel->data + sizeOfCut * i, sizeOfCut, &results[i]);
    }

    for (int i = 0; i < rawModel->numCuts; i++) {
        worker[i].join();
    }

    histogram = std::accumulate(results.begin(), results.end(), histogram, Work::sumArrays);
    float maxHistValue = *std::max_element(histogram.begin(), histogram.end());
    std::transform(histogram.begin(), histogram.end(), histogram.begin(), std::bind1st(std::multiplies<float>(), 1 / maxHistValue));
    isHistLoaded = true;
}

bool EditingWindow::_isMouseOver(EditingWindow * eWin, sf::CircleShape &circle)
{
    sf::Vector2f mousePos(sf::Mouse::getPosition(*eWin->window));

    if (mousePos.x > circle.getPosition().x - DRAG_TOLERANCE &&
            mousePos.x <= circle.getPosition().x + circle.getGlobalBounds().width + DRAG_TOLERANCE &&
            mousePos.y > circle.getPosition().y - DRAG_TOLERANCE &&
            mousePos.y <= circle.getPosition().y + circle.getGlobalBounds().height + DRAG_TOLERANCE
       ) {
        return true;
    } else {
        return false;
    }
}

void EditingWindow::_updateTransferFunction(EditingWindow * eWin)
{
    if (sf::Mouse::getPosition(*eWin->window).y > 2 && sf::Mouse::getPosition(*eWin->window).x > 0 &&
            sf::Mouse::getPosition(*eWin->window).x < 769 && sf::Mouse::getPosition(*eWin->window).y < 259) {
        TransferFunction::addControlPoint(255, 255, 255, 255 - sf::Mouse::getPosition(*eWin->window).y + 3,
                                          sf::Mouse::getPosition(*eWin->window).x / 3);
    }
}

void EditingWindow::_initEditingWindowUI(UIBuilder &ui, EditingWindow * eWin)
{
    ui.addBar("Points");
    ui.setBarSize("Points", 200, 200);
    ui.setBarPosition("Points", eWin->parent->getSize().x - 205, 5);

    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
        ui.addColorControls("Points", "Point " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
    }
}

void EditingWindow::_initRenderContext(EditingWindow * eWin)
{
    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 8;
    settings.majorVersion = 4;
    settings.minorVersion = 0;
    eWin->window = new sf::RenderWindow(sf::VideoMode(775, 285, 32), "Funcion de Transferencia", sf::Style::Titlebar, settings);
    eWin->window->setPosition(sf::Vector2i(0, 0));
}

void EditingWindow::_drawHistogram(sf::RectangleShape &line, EditingWindow * eWin, sf::RectangleShape &indicator)
{
    for (int i = 0; i < 256; i++) {
        // Histogram Values
        line.setSize(sf::Vector2f(log10(eWin->histogram[i] * 9 + 1) * 256.0f, 2));
        line.setPosition(5 + i * 3, 260);
        eWin->window->draw(line);
        /// Transfer Function Result
        GLubyte * color = eWin->rawModel->transferFunc[i];
        indicator.setSize(sf::Vector2f(3, 10));
        indicator.setPosition(5 + i * 3, 273);
        indicator.setFillColor(sf::Color((int) * (color), (int) * (color + 1), (int) * (color + 2), (int) * (color + 3)));
        eWin->window->draw(indicator);
        // Iso Value Indicators
        indicator.setSize(sf::Vector2f(2, 10));
        indicator.setPosition(5 + i * 3, 260);
        indicator.setFillColor(sf::Color(i, i, i, i));
        eWin->window->draw(indicator);
    }
}

void EditingWindow::_drawControlPointCircles(int i, sf::CircleShape &circle, EditingWindow * eWin)
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
    circle.setPosition(TransferFunction::getControlPoints()[i].isoValue * 3 - 3, 255 - TransferFunction::getControlPoints()[i].rgba[3] * 255);

    if (_isMouseOver(eWin, circle)) {
        circle.setOutlineColor(sf::Color::Green);

        if (eWin->mouseOverIndex == -1) { eWin->mouseOverIndex = i; }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && i == eWin->mouseOverIndex) {
            if (!eWin->dragStarted) {
                eWin->dragStarted = true;
            } else {
                circle.setOutlineThickness(2);
                int finalIsoValue = sf::Mouse::getPosition(*eWin->window).x / 3;
                int finalAlphaValue = 255 - sf::Mouse::getPosition(*eWin->window).y + 3;
                TransferFunction::deleteAlphaControlPoint(i);
                finalIsoValue = i == 0 ? 0 : i == TransferFunction::getControlPoints().size() ? 255 : finalIsoValue;
                TransferFunction::addControlPoint(rgba.r, rgba.g, rgba.b, finalAlphaValue, finalIsoValue);
            }
        } else if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && i == eWin->mouseOverIndex) {
            if (i > 0 && i < TransferFunction::getControlPoints().size() - 1) {
                TransferFunction::deleteAlphaControlPoint(i);
            }
        } else {
            eWin->mouseOverIndex = -1;
            eWin->dragStarted = false;
        }
    }

    eWin->window->draw(circle);
}

void EditingWindow::_drawTransferFuncPlot(int i, sf::CircleShape &circle, EditingWindow * eWin)
{
    // Plotting lines
    if (i < TransferFunction::getControlPoints().size() - 1) {
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
}

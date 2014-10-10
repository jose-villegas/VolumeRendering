#include "EditingWindow.h"


EditingWindow::EditingWindow(void)
{
    histogram.fill(0);
    rawModel = NULL;
    windowThread = NULL;
    parent = window = NULL;
    bool isHistLoaded = false;

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
    eWin->window = new sf::RenderWindow(sf::VideoMode(775, 270, 32), "Funcion de Transferencia", sf::Style::Titlebar);
    eWin->window->setPosition(sf::Vector2i(0, 0));
    sf::RectangleShape indicator[256];

    for (int i = 0; i < 256; i++)
    {
        indicator[i] = sf::RectangleShape(sf::Vector2f(2, 6));
        indicator[i].setFillColor(sf::Color(i, i, i, i));
        indicator[i].setPosition(5 + i * 3, 265);
    }

    // Histogram

    while (eWin->parent->isOpen() && eWin->window->isOpen())
    {
        sf::Event event;

        while (eWin->window->pollEvent(event))
        {
        }

        eWin->window->clear(sf::Color::Transparent);

        for (int i = 0; i < 256; i++)
        {
            if (eWin->isHistLoaded)
            {
                eWin->line[i].setSize(sf::Vector2f(log10(eWin->histogram[i] * 9 + 1) * 256.0f, 2));
                eWin->window->draw(eWin->line[i]);
            }

            eWin->window->draw(indicator[i]);
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


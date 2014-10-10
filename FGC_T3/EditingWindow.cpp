#include "EditingWindow.h"


EditingWindow::EditingWindow(void)
{
    histogram.fill(0);
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
    eWin->window = new sf::RenderWindow(sf::VideoMode(300, 300, 32), "Funcion de Transferencia", sf::Style::Titlebar);
    eWin->window->setPosition(sf::Vector2i(0, 0));

    while (eWin->parent->isOpen() && eWin->window->isOpen())
    {
        sf::Event event;

        while (eWin->window->pollEvent(event))
        {
        }

        eWin->window->clear(sf::Color::Black);
        eWin->window->display();
    }

    return;
}

void EditingWindow::loadHistogram(RawDataModel * rawModel)
{
    this->rawModel = rawModel;
    std::vector<std::thread> worker(rawModel->numCuts);
    std::vector<std::array<int, 256>> results(rawModel->numCuts);
    struct Work
    {
        static void cutHistogram(GLubyte * cut, int size, std::array<int, 256> * result)
        {
            for (int i = 0; i < size; i++)
            {
                result->at((int)(*(cut + i))) += 1;
            }

            return;
        }

        static std::array<int, 256> sumArrays(std::array<int, 256> a, std::array<int, 256> b)
        {
            std::array<int, 256> result;
            std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::plus<int>());
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
}


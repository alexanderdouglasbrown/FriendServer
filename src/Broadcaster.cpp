#include "Broadcaster.h"

Broadcaster *Broadcaster::instance = nullptr;

Broadcaster::Broadcaster()
{
}

Broadcaster *Broadcaster::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Broadcaster();
    }

    return instance;
}
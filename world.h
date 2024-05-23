#ifndef WORLD
#define WORLD

#include <iostream>

struct location
{
    std::string tag, description;
};

struct path
{
    location *locs[2];
};

#endif
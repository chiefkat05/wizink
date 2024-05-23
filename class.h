#ifndef CLASS_H
#define CLASS_H

#include "world.h"

struct baseChar
{
    const char *tag;
    int speed, weight;
    bool dead;
    location *current_location;
};

#endif
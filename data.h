#ifndef DATA_H
#define DATA_H

#include "world.h"
#include "class.h"

location locs[] = {
    {"dungeon", "cobblestone, cobwebs, and wizards?"},
    {"stairway", "not great for fighting on, but cinematic!"},
    {"upper dungeon", "less cobwebs, more wizards"}};

path paths[] = {
    {&locs[0], &locs[1]},
    {&locs[1], &locs[2]}};

void lookNearby(location *currentLoc)
{
    for (int i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
    {
        if (currentLoc == paths[i].locs[0] && currentLoc != paths[i].locs[1])
        {
            std::cout << paths[i].locs[1]->description << std::endl;
        }
        if (currentLoc == paths[i].locs[1] && currentLoc != paths[i].locs[0])
        {
            std::cout << paths[i].locs[0]->description << std::endl;
        }
    }
}

// yes I'm proud of this
void force(baseChar *parent, baseChar *inflictor)
{
    // this is assuming inflictor has already gained momentum and
    // their weight is helping their speed while parent needs to react and so
    // all the weight they need to move is working against them.
    // Remember to add some 1-turn bonuses to speed for preparedness and such
    std::cout << "hit check: " << inflictor->speed << " + " << inflictor->weight << " = "
              << inflictor->speed + inflictor->weight << " vs " << parent->speed << " - "
              << parent->weight << " = " << parent->speed - parent->weight << "\n";
    if (inflictor->speed + inflictor->weight < parent->speed - parent->weight)
    {
        std::cout << parent->tag << " swiftly dodges the attack!\n";
        return;
    }
    // if the inflictor force (speed * weight) is more than twice as large as the
    // parent weight, minus parent speed for 'shifting body so that the hit is not lethal'
    // then the parent is die harded
    std::cout << "death check: " << inflictor->weight << " * " << inflictor->speed << " / " << 2 << " - "
              << parent->speed << " = " << inflictor->weight * inflictor->speed / 2 - parent->speed
              << " vs " << parent->weight << "\n";
    if (inflictor->weight * inflictor->speed / 2 - parent->speed > parent->weight)
    {
        parent->dead = true;
        std::cout << parent->tag << " has died of lethal force.\n";
        return;
    }
    std::cout << "nodmg check: " << inflictor->weight << " * " << inflictor->speed
              << " - " << parent->speed << " = "
              << inflictor->weight * inflictor->speed - parent->speed
              << " vs " << parent->weight << "\n";
    if (inflictor->weight * inflictor->speed - parent->speed < parent->weight)
    {
        std::cout << parent->tag << " is not even fazed!\n";
        return;
    }
    std::cout << parent->tag << " eats the hit! They look a little worse off...\n";
    parent->speed -= inflictor->weight;
    std::cout << parent->tag << "'s speed is now " << parent->speed << "\n";
}
void alert(baseChar *parent)
{
    std::cout << parent->tag << " watches for their next move...\n";
    parent->speed += 2;
}

// status force((event *)force);

#endif
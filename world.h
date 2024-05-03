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

#endif
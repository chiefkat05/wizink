//  Developed over the course of a couple painful minutes by 'Chiefkat05'
//  sometimes going by 'Chiefkat' or 'KatzOverlord'.
//  If there's a purple cartoon beetle as the user's profile icon, it's probably me.
//  You should have gotten this from my official website at:
//  https:\\wixsite.chiefkat05.beetlespagetti/home
//  If you didn't, that doesn't mean the place you got it broke the website or anything,
//  (see license,) but I would love to know if someone is interested in what I make :P
//          -- Chiefkat05

//  Copyright (C) 2024 Chiefkat05

//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "multiplayer.h"

enum class CustomMsgTypes : uint32_t
{
    // implement locations that have positional points within them
    FORCE,
    MOVE,
    ALERT
};

class WizClient : public client_interface<CustomMsgTypes>
{
public:
    bool cforce(baseChar *parent, baseChar *inflictor)
    {
        message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::FORCE;
        msg << parent << inflictor;
        // MessageClient(msg);

        return true;
    }
};
class WizServer : public server_interface<CustomMsgTypes>
{
public:
    WizServer(uint16_t nPort) : server_interface<CustomMsgTypes>(nPort)
    {
    }

protected:
    virtual bool OnClientConnect(std::shared_ptr<connection<CustomMsgTypes>> client)
    {
        return true;
    }

    virtual void OnClientDisconnect(std::shared_ptr<connection<CustomMsgTypes>> client)
    {
    }

    virtual void OnMessage(std::shared_ptr<connection<CustomMsgTypes>> client, message<CustomMsgTypes> &msg)
    {
    }
};

class CustomServer : public server_interface<CustomMsgTypes>
{
public:
    CustomServer(uint16_t nPort) : server_interface<CustomMsgTypes>(nPort) {}

protected:
    virtual bool OnClientConnect(std::shared_ptr<connection<CustomMsgTypes>> client)
    {
        return true;
    }

    virtual void OnClientDisconnect(std::shared_ptr<connection<CustomMsgTypes>> client)
    {
    }

    virtual void OnMessage(std::shared_ptr<connection<CustomMsgTypes>> client, message<CustomMsgTypes> msg)
    {
    }
};

int main()
{
    CustomServer server(60000);
    server.Start();

    while (1)
    {
        server.Update();
    }
    // std::string input;

    // baseChar player = {"The player", 4, 2, false, &locs[0]};

    // lookNearby(player.current_location);

    // WizServer server(60000); // just pretend it works and keep going I guess
    // then give up on the retarded fuckery and just go with the official documentation
    // server.Start();

    // while (true)
    // {
    //     server.Update();
    // }

    // server();

    // std::cout << "You are a 'wizard', trained and equipped with a reality-bending engine which modern technology (as far as you know,) can scarcely compete with.\n"
    //              "Having grown and lived among fellow magic-wielders in a horrid cult, you've now decided to make your move and escape.\n\n";
    // std::cout << "You are stopped by a fellow magic user. His nickname is 'speedy'. Most of the magicians find him annoying and rather pathetic.\n"
    //              "\"You won't even see me coming!\" Let's get this over with.\n\n";

    // baseChar player = {"The player", 4, 2, false};
    // baseChar speedy = {"Speedy", 6, 1, false};
    // baseChar mbullet = {"The magic bullet", 8, 3, false};
    // baseChar playerwall = {"The magic wall", 0, 12, false};

    // std::cout << "Speedy hits you with his fist! Very primitive.\n";
    // force(&player, &speedy);
    // std::cout << "Speedy swings around with his other fist! Very neanderthalean.\n";
    // force(&player, &speedy);

    // std::cout << "Speedy sends a magic missle careening towards you! You pull up a magical wall to sheild yourself.\n";

    // force(&playerwall, &mbullet);

    // std::cout << "\nPlease select your next course of action:\n\n";
    // std::cout << "--> punch\n";
    // std::cout << "--> missile\n";
    // std::cout << "--> prepare\n";

    // std::string input;
    // while (std::getline(std::cin, input))
    // {
    //     if (input == "punch")
    //     {
    //         force(&speedy, &player);
    //         goto end;
    //     }
    //     if (input == "missile")
    //     {
    //         force(&speedy, &mbullet);
    //         goto end;
    //     }
    //     if (input == "prepare")
    //     {
    //         alert(&player);
    //         goto end;
    //     }

    //     std::cout << "You can't decide on what to do and end up having a brain implosion. -1 turn.\n";

    // end:
    //     if (player.dead || speedy.dead)
    //         break;

    //     std::cout << "\nSpeedy hits you with his fist! Very primitive.\n";
    //     force(&player, &speedy);

    //     std::cout << "\nPlease select your next course of action:\n\n";
    //     std::cout << "--> punch\n";
    //     std::cout << "--> missile\n";
    //     std::cout << "--> prepare\n";
    // }
    // if (player.dead)
    //     std::cout << "\nGame over.\n";

    // if (speedy.dead)
    //     std::cout << "\nSpeedy will probably be forgotten\n";

    return 0;
}
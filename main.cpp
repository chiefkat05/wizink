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
#include "data.h"

enum mHeaders
{
    PING,
    CHAT
};

int main()
{
    std::string input;

    std::cout << "type 'server' to open a server on port 4444\n"
                 "or type 'client' to attempt a client connection to 127.0.0.1/4444\n--> ";

    message msg;

    msg.header.id = mHeaders::PING;

    int f = 549;
    double d = 75.36;

    msg << f << d;

    f = 2;
    d = 2.0f;

    msg >> d >> f;

    std::cin >> input;
    if (input == "server")
    {
        server wizServer(4444);
        wizServer.Start();

        while (1)
        {
            wizServer.Update(-1);
        }
    }
    if (input == "client")
    {
        std::cout << "please enter the ip address of the server you would like to connect to:\n-->";
        std::cin >> input;
        client cl;
        cl.Connect(input, "4444");
        std::array<char, 128> msgInput;

        std::thread readThr([&cl]()
                            {
                                while (1)
                                {
                                    if (!cl.Incoming().empty())
                                    {
                                        auto msg = cl.Incoming().pop_front().msg;

                                        switch(msg.header.id)
                                        {
                                            case 0:
                                            std::cout << "server message - " << msg.body.data() << "\n";
                                        break;
                                        case 1:
                                            uint32_t cID;
                                            // std::cout << msg.body.size() << ", " << (int)msg.body.data() << ", " << cID << " huh\n";
                                            msg >> cID;
                                            // std::cout << msg.body.size() << ", " << cID << " huh2\n";
                                            std::cout << "hello from client " << std::to_string(cID) << "\n";
                                            break;
                                        default:
                                            std::cout << "received unknown message type\n";
                                            break;
                                        }
                                    }
                                } });

        while (msgInput.data() != "quit")
        {
            if (!cl.IsConnected())
                break;

            std::cin >> msgInput.data();
            if (msgInput.front() == '@')
            {
                cl.MessageAll(msgInput);
            }
            else
            {
                cl.SendMessage(msgInput);
            }
        }

        cl.Disconnect();

        if (readThr.joinable())
            readThr.join();
    }

    // baseChar player = {"The player", 4, 2, false, &locs[0]};

    // lookNearby(player.current_location);

    // while (true)
    // {
    //     server.Update();
    // }

    // server();

    // std::cout << "You are a 'wizard', trained and equipped with a reality-bending engine which modern technology (as far as you know,) can scarcely compete with.\n"
    //              "Having grown and lived among fellow magic-wielders in a horrid cult, you've now decided to make your move and escape.\n\n";
    // std::cout << "You are stopped by a fellow magic user. His nickname is 'speedy'. Most of the magicians find him annoying and rather patheticlient.\n"
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
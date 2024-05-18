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

// enum class CustomMsgTypes : uint32_t
// {
//     ServerAccept,
//     ServerDeny,
//     ServerPing,
//     MessageAll,
//     ServerMessage,
//     Chat
// };
// class WizClient : public client_interface<CustomMsgTypes>
// {
// public:
//     void PingServer()
//     {
//         message<CustomMsgTypes> msg;
//         msg.header.id = CustomMsgTypes::ServerPing;

//         // Caution with this...
//         std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

//         msg << timeNow;
//         Send(msg);
//     }

//     void MessageAll()
//     {
//         message<CustomMsgTypes> msg;
//         msg.header.id = CustomMsgTypes::MessageAll;
//         Send(msg);
//     }
//     void Chat(std::string input)
//     {
//         message<CustomMsgTypes> msg;
//         msg.header.id = CustomMsgTypes::Chat;
//         msg << "Client says: " << input;
//         Send(msg);
//     }
// };

// class WizServer : public server_interface<CustomMsgTypes>
// {
// public:
//     WizServer(uint16_t nPort) : server_interface<CustomMsgTypes>(nPort)
//     {
//     }

// protected:
//     virtual bool OnClientConnect(std::shared_ptr<connection<CustomMsgTypes>> client)
//     {
//         message<CustomMsgTypes> msg;
//         msg.header.id = CustomMsgTypes::ServerAccept;
//         client->Send(msg);
//         return true;
//     }

//     // Called when a client appears to have disconnected
//     virtual void OnClientDisconnect(std::shared_ptr<connection<CustomMsgTypes>> client)
//     {
//         std::cout << "Removing client [" << client->GetID() << "]\n";
//     }

//     // Called when a message arrives
//     virtual void OnMessage(std::shared_ptr<connection<CustomMsgTypes>> client, message<CustomMsgTypes> &msg)
//     {
//         switch (msg.header.id)
//         {
//         case CustomMsgTypes::ServerPing:
//         {
//             std::cout << "[" << client->GetID() << "]: Server Ping\n";

//             // Simply bounce message back to client
//             client->Send(msg);
//         }
//         break;

//         case CustomMsgTypes::MessageAll:
//         {
//             std::cout << "[" << client->GetID() << "]: Message All\n";

//             // Construct a new message and send it to all clients
//             message<CustomMsgTypes> msg;
//             msg.header.id = CustomMsgTypes::ServerMessage;
//             msg << client->GetID();
//             MessageAllClients(msg, client);
//         }
//         break;

//         case CustomMsgTypes::Chat:
//         {
//             std::string output;
//             msg >> output;
//             std::cout << "[" << client->GetID() << "]: " << output << "\n";
//         }
//         }
//     }
// };

int main()
{
    std::string input;

    std::cout << "type 'server' to open a server on port 4444\n"
                 "or type 'client' to attempt a client connection to 127.0.0.1/4444\n";

    std::cin >> input;
    if (input == "server")
    {
        // WizServer server(4444);
        // server.Start();

        // while (1)
        // {
        //     server.Update(-1, true);
        // }
    }
    if (input == "client")
    {
        // WizClient client;
        // client.Connect("127.0.0.1", 4444);

        // std::cout << "type 'quit' when you want to quit, or send a message to the server with anything else.\n";
        // while (std::cin >> input && input != "quit")
        // {
        //     // client.Send(input);
        //     // message<CustomMsgTypes> msg;
        //     // msg.header.id = CustomMsgTypes::ServerPing;
        //     // client.MessageAll();
        //     std::cout << input << " is the current input\n";
        //     if (input == "ping")
        //     {
        //         client.PingServer();
        //     }
        //     if (input == "mAll")
        //     {
        //         client.MessageAll();
        //     }
        //     if (input == "chat")
        //     {
        //         client.Chat(input);
        //     }
        //     input = "";

        //     if (client.IsConnected())
        //     {
        //         if (!client.Incoming().empty())
        //         {

        //             auto msg = client.Incoming().pop_front().msg;

        //             switch (msg.header.id)
        //             {
        //             case CustomMsgTypes::ServerAccept:
        //             {
        //                 // Server has responded to a ping request
        //                 std::cout << "Server Accepted Connection\n";
        //             }
        //             break;

        //             case CustomMsgTypes::ServerPing:
        //             {
        //                 // Server has responded to a ping request
        //                 std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
        //                 std::chrono::system_clock::time_point timeThen;
        //                 msg >> timeThen;
        //                 std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
        //             }
        //             break;

        //             case CustomMsgTypes::ServerMessage:
        //             {
        //                 // Server has responded to a ping request
        //                 uint32_t clientID;
        //                 msg >> clientID;
        //                 std::cout << "Hello from [" << clientID << "]\n";
        //             }
        //             break;
        //             }
        //         }
        //     }
        //     else
        //     {
        //         std::cout << "Server Down\n";
        //         return -1;
        //     }

        // msg << input;
        // client.Send(msg);
        // if (!client.Incoming().empty())
        // {
        //     auto msg = client.Incoming().pop_front().msg;

        //     std::cout << msg << "is the message";
        // }
    }
    // }
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
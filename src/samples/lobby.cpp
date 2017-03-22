#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

using namespace std::chrono_literals;

#include "tunnler/lobby.h"

const std::string MasterServer = "http://127.0.0.1:8000/"; //FIXME: Use

int main(int argc, char* argv[]) {
    std::cout << "Lobby example" << std::endl;

    Lobby lobby;

    auto ListRooms = [&]() {
        std::cout << "Retrieving rooms" << std::endl;

        std::vector<Lobby::FoundRoom> rooms = lobby.GetRooms();
        for(auto& room : rooms) {
            std::cout << "Found room: \"" << room.server << "\" (Port " << room.server_port << ")" << std::endl;
            if (!room.information) {
              room.information = std::make_shared<Lobby::RoomInformation>(lobby.GetRoomInformation(room.server, room.server_port));
              std::cout << "Querying information.." << std::endl;
            } else {
              //FIXME: We already have the info!
              std::cout << "Information:" << std::endl;
            }
        }
    };

#if 0

    std::string token_a = lobby.CreateRoom("server-a", 1234);
    std::cout << "Created room \"" << token_a << "\"" << std::endl;

    std::string token_b = lobby.CreateRoom("server-b", 1234);
    std::cout << "Created room \"" << token_b << "\"" << std::endl;

    ListRooms();

    std::cout << "Destroyed? " << lobby.DestroyRoom(token_a) << std::endl;

    ListRooms();
#endif

    std::vector<std::string> args(argv, argv + argc);

    if (args.size() >= 2) {

        std::string command = args[1];
        if (command == "list") {

            // Do some LAN discovery
            uint16_t server_port = 1234; //atoi(args[2].c_str());
            std::cout << "Starting scan" << std::endl;
            lobby.StartScan(server_port);
            std::this_thread::sleep_for(1s);
            std::cout << "Stopping scan" << std::endl;

lobby.updateFoundRooms();

            lobby.StopScan();
/*
            for(auto& room : rooms) {
                if (!room.information) {
                    room.information = std::make_shared(lobby.GetRoomInformation(room));
                }
            }
*/
            std::this_thread::sleep_for(1s);

            ListRooms();
        } else if (command == "create") {
            std::string server = args[2];
            uint16_t server_port = atoi(args[3].c_str());
            std::string token = lobby.CreateRoom(server, server_port);
            std::cout << "Token: \"" << token << "\"" << std::endl;
        } else if (command == "destroy") {
            std::string token = args[2];
            bool status = lobby.DestroyRoom(token);
            std::cout << "Status: " << (status ? "Success" : "Failure") << std::endl;
        } else if (command == "update") {
            std::string token = args[2];
            bool status = lobby.UpdateRoom(token);
            std::cout << "Status: " << (status ? "Success" : "Failure") << std::endl;
        } else {
            std::cerr << "Unknown command \"" << args[0] << "\"" << std::endl;
        }

    }

    return 0;
}

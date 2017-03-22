#include <iostream>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#include "tunnler/room.h"

int main(int argc, char* argv[]) {
    std::cout << "Server example" << std::endl;

    std::vector<std::string> args(argv, argv + argc);

    if (args.size() < 2) {
        std::cerr << args[0] << " <name>" << std::endl;
        return 1;
    }

    std::string name = args[1];

    Room room;
    room.Create(name);

    while(true) {
      std::cout << "Okay" << std::endl;
      std::this_thread::sleep_for(2s);
    }

    room.Destroy();   

    return 0;
}

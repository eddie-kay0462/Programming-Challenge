#include "transport.h"

int main(int argc, char* argv[]) {
    string file = "ghana_cities_graph_2026.txt";
    if (argc > 1) file = argv[1];

    TransportGraph G;
    if (!G.loadFromFile(file)) return 1;

    G.run();
    return 0;
}

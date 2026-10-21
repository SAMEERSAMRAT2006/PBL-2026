// citymap.cpp
// The city is stored as a weighted graph.
// Dijkstra is used to find the shortest route.
#include "citymap.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <limits>

using namespace std;

static const string locationNames[NUM_LOCATIONS] = {
    "Hospital",
    "Fire Station",
    "Police Station",
    "Market",
    "School",
    "Highway Junction"
};

static int roadMap[NUM_LOCATIONS][NUM_LOCATIONS];

void loadCityMap() {
    int roads[8][3] = {
        {0, 1, 4},
        {0, 2, 2},
        {1, 3, 5},
        {2, 3, 8},
        {2, 4, 10},
        {3, 4, 2},
        {3, 5, 6},
        {4, 5, 3}
    };

    for (int i = 0; i < NUM_LOCATIONS; i++)
        for (int j = 0; j < NUM_LOCATIONS; j++)
            roadMap[i][j] = 0;

    for (int i = 0; i < 8; i++) {
        int a = roads[i][0];
        int b = roads[i][1];
        int distance = roads[i][2];

        roadMap[a][b] = distance;
        roadMap[b][a] = distance;
    }
}

void displayMap() {
    cout << "\n========== CITY MAP ==========\n";

    for (int i = 0; i < NUM_LOCATIONS; i++)
        cout << i << " = " << locationNames[i] << "\n";

    cout << "\nRoad distances (km):\n";

    for (int i = 0; i < NUM_LOCATIONS; i++) {
        for (int j = i + 1; j < NUM_LOCATIONS; j++) {
            if (roadMap[i][j] != 0) {
                cout << locationNames[i] << " <-> "
                     << locationNames[j] << " : "
                     << roadMap[i][j] << " km\n";
            }
        }
    }

    cout << "==============================\n";
}

string getLocationName(int id) {
    if (id < 0 || id >= NUM_LOCATIONS)
        return "Unknown";

    return locationNames[id];
}

int getRoadDistance(int from, int to) {
    if (from < 0 || from >= NUM_LOCATIONS ||
        to < 0 || to >= NUM_LOCATIONS)
        return -1;

    return roadMap[from][to];
}

int getDistance(int from, int to) {
    if (from < 0 || from >= NUM_LOCATIONS ||
        to < 0 || to >= NUM_LOCATIONS)
        return -1;

    // Phase II uses the direct road distance only.
    // Shortest-path calculation will be added in the next phase.
    return roadMap[from][to] == 0 ? -1 : roadMap[from][to];
}

int getNextLocation(int from, int to) {
    if (from < 0 || from >= NUM_LOCATIONS ||
        to < 0 || to >= NUM_LOCATIONS)
        return -1;

    if (roadMap[from][to] == 0)
        return -1;

    return to;
}

void showShortestPath(int from, int to) {
    if (from < 0 || from >= NUM_LOCATIONS ||
        to < 0 || to >= NUM_LOCATIONS) {
        cout << "Invalid location.\n";
        return;
    }

    if (from == to) {
        cout << "Already at " << locationNames[from] << ".\n";
        return;
    }

    if (roadMap[from][to] == 0) {
        cout << "No direct road available between these locations.\n";
        return;
    }

    cout << "Route: " << locationNames[from]
         << " -> " << locationNames[to] << "\n";
    cout << "Road distance: " << roadMap[from][to] << " km\n";
}

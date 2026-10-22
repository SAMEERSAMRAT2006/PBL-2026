// citymap.cpp - Part 1: City Map (Paras Chauhan)
// The city is a small weighted graph kept in a 2D array.
// Shortest distances and "which road to take first" were worked out in
// advance and stored in two fixed tables, so NO search algorithm runs
// in this phase (no BFS, no Dijkstra) - the program only looks up numbers.
#include "ers.h"

#include <iostream>
#include <vector>

using namespace std;

static const string locationNames[NUM_LOCATIONS] = {
    "Hospital", "Fire Station", "Police Station",
    "Market", "School", "Highway Junction"
};

// Direct roads in km (0 = no road). Filled by loadCityMap().
static int roadMap[NUM_LOCATIONS][NUM_LOCATIONS];

// Shortest distance between every pair of places (fixed, preloaded).
static const int distanceTable[NUM_LOCATIONS][NUM_LOCATIONS] = {
    { 0,  4,  2,  9, 11, 14},
    { 4,  0,  6,  5,  7, 10},
    { 2,  6,  0,  8, 10, 13},
    { 9,  5,  8,  0,  2,  5},
    {11,  7, 10,  2,  0,  3},
    {14, 10, 13,  5,  3,  0}
};

// nextStopTable[a][b] = the place to drive to FIRST when going from a to b.
static const int nextStopTable[NUM_LOCATIONS][NUM_LOCATIONS] = {
    {0, 1, 2, 1, 1, 1},
    {0, 1, 0, 3, 3, 3},
    {0, 0, 2, 3, 4, 4},
    {1, 1, 2, 3, 4, 4},
    {3, 3, 2, 3, 4, 5},
    {4, 4, 4, 4, 4, 5}
};

static bool validPlace(int id) {
    return id >= 0 && id < NUM_LOCATIONS;
}

// Function 1: fill the road matrix (every road works both ways).
void loadCityMap() {
    int roads[8][3] = {
        {0, 1, 4}, {0, 2, 2}, {1, 3, 5}, {2, 3, 8},
        {2, 4, 10}, {3, 4, 2}, {3, 5, 6}, {4, 5, 3}
    };

    for (int i = 0; i < NUM_LOCATIONS; i++)
        for (int j = 0; j < NUM_LOCATIONS; j++)
            roadMap[i][j] = 0;

    for (int i = 0; i < 8; i++) {
        roadMap[roads[i][0]][roads[i][1]] = roads[i][2];
        roadMap[roads[i][1]][roads[i][0]] = roads[i][2];
    }
}

// ----- helpers that draw the picture on a grid of characters -----

static void put(vector<string> &grid, int row, int col, const string &text) {
    for (size_t i = 0; i < text.size() && col + i < grid[row].size(); i++)
        grid[row][col + i] = text[i];
}

static int nodeWidth(int id) {
    return 4 + static_cast<int>(locationNames[id].size());      // "[3] Market"
}

// road between two places on the same row, e.g. "---- 5 ----"
// it starts just after place a's name and stops just before the next column
static void horizontalRoad(vector<string> &grid, int row, int col, int a, int b, int nextCol) {
    int start = col + nodeWidth(a) + 1;
    int length = nextCol - 1 - start;
    string line(length, '-');
    string label = " " + to_string(roadMap[a][b]) + " ";
    line.replace((length - label.size()) / 2, label.size(), label);
    put(grid, row, start, line);
}

// road between the top row and the bottom row
static void verticalRoad(vector<string> &grid, int col, int a, int b) {
    for (int r = 2; r <= 5; r++)
        put(grid, r, col + 8, "|");
    put(grid, 3, col + 7, " " + to_string(roadMap[a][b]) + " ");
}

static void node(vector<string> &grid, int row, int col, int id, const string *marks) {
    put(grid, row, col, "[" + to_string(id) + "] " + locationNames[id]);
    if (marks != nullptr)
        put(grid, row + 1, col, marks[id]);
}

// Function 2: draw the city as a picture.
// marks[i] is a short text shown under place i (teams and open emergencies).
void displayMap(const string *marks) {
    vector<string> grid(8, string(78, ' '));
    int c0 = 0, c1 = 28, c2 = 56;      // left, middle and right columns

    node(grid, 0, c0, 1, marks);   node(grid, 0, c1, 3, marks);   node(grid, 0, c2, 5, marks);
    node(grid, 6, c0, 0, marks);   node(grid, 6, c1, 2, marks);   node(grid, 6, c2, 4, marks);

    horizontalRoad(grid, 0, c0, 1, 3, c1);   horizontalRoad(grid, 0, c1, 3, 5, c2);
    horizontalRoad(grid, 6, c0, 0, 2, c1);   horizontalRoad(grid, 6, c1, 2, 4, c2);

    verticalRoad(grid, c0, 1, 0);        // Fire Station - Hospital
    verticalRoad(grid, c1, 3, 2);        // Market - Police Station
    verticalRoad(grid, c2, 5, 4);        // Highway Junction - School

    // diagonal road Market - School
    for (int r = 2; r <= 5; r++)
        put(grid, r, 44 + 3 * (r - 2), r == 3 ? to_string(roadMap[3][4]) : "\\");

    cout << "\n=============================== CITY MAP ===============================\n\n";
    for (size_t r = 0; r < grid.size(); r++) {
        size_t last = grid[r].find_last_not_of(' ');          // cut trailing spaces
        cout << (last == string::npos ? "" : grid[r].substr(0, last + 1)) << "\n";
    }
    cout << "\nNumbers on the roads are km.\n";
    if (marks != nullptr) {
        cout << "A = Ambulance, F = Fire truck, P = Police car  (* = team is busy)\n";
        cout << "!n = emergency number n is still open at that place\n";
    }
    cout << "========================================================================\n";
}

string getLocationName(int id) {
    return validPlace(id) ? locationNames[id] : "Unknown";
}

// Function 5 (part): look up the shortest distance in the table.
int getDistance(int from, int to) {
    if (!validPlace(from) || !validPlace(to))
        return -1;
    return distanceTable[from][to];
}

int getNextStop(int from, int to) {
    if (!validPlace(from) || !validPlace(to))
        return -1;
    return nextStopTable[from][to];
}

// "Hospital -> Fire Station -> Market" by following the next-stop table.
string routeText(int from, int to) {
    if (!validPlace(from) || !validPlace(to))
        return "Unknown";

    string text = locationNames[from];
    int at = from;
    while (at != to) {
        at = nextStopTable[at][to];
        text += " -> " + locationNames[at];
    }
    return text;
}

// Function 5: print the route, distance and travel time.
void showShortestPath(int from, int to) {
    if (!validPlace(from) || !validPlace(to)) {
        cout << "Invalid location.\n";
        return;
    }
    if (from == to) {
        cout << "Already at " << locationNames[from] << ".\n";
        return;
    }
    cout << "\nRoute    : " << routeText(from, to) << "\n";
    cout << "Distance : " << distanceTable[from][to] << " km\n";
    cout << "Travel   : about " << distanceTable[from][to] * MIN_PER_KM << " minutes\n";
}

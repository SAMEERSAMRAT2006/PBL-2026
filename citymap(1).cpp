
#include "ers.h"

#include <iostream>
#include <vector>

using namespace std;

static const string locationNames[NUM_LOCATIONS] = {
    "Hospital", "Fire Station", "Police Station",
    "Market", "School", "Highway Junction"
};


static int roadMap[NUM_LOCATIONS][NUM_LOCATIONS];


static const int distanceTable[NUM_LOCATIONS][NUM_LOCATIONS] = {
    { 0,  4,  2,  9, 11, 14},
    { 4,  0,  6,  5,  7, 10},
    { 2,  6,  0,  8, 10, 13},
    { 9,  5,  8,  0,  2,  5},
    {11,  7, 10,  2,  0,  3},
    {14, 10, 13,  5,  3,  0}
};


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



static void put(vector<string> &grid, int row, int col, const string &text) {
    for (size_t i = 0; i < text.size() && col + i < grid[row].size(); i++)
        grid[row][col + i] = text[i];
}

static int nodeWidth(int id) {
    return 4 + static_cast<int>(locationNames[id].size());      
}


static void horizontalRoad(vector<string> &grid, int row, int col, int a, int b, int nextCol) {
    int start = col + nodeWidth(a) + 1;
    int length = nextCol - 1 - start;
    string line(length, '-');
    string label = " " + to_string(roadMap[a][b]) + " ";
    line.replace((length - label.size()) / 2, label.size(), label);
    put(grid, row, start, line);
}


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


void displayMap(const string *marks) {
    vector<string> grid(8, string(78, ' '));
    int c0 = 0, c1 = 28, c2 = 56;      

    node(grid, 0, c0, 1, marks);   node(grid, 0, c1, 3, marks);   node(grid, 0, c2, 5, marks);
    node(grid, 6, c0, 0, marks);   node(grid, 6, c1, 2, marks);   node(grid, 6, c2, 4, marks);

    horizontalRoad(grid, 0, c0, 1, 3, c1);   horizontalRoad(grid, 0, c1, 3, 5, c2);
    horizontalRoad(grid, 6, c0, 0, 2, c1);   horizontalRoad(grid, 6, c1, 2, 4, c2);

    verticalRoad(grid, c0, 1, 0);        
    verticalRoad(grid, c1, 3, 2);        
    verticalRoad(grid, c2, 5, 4);        

    
    for (int r = 2; r <= 5; r++)
        put(grid, r, 44 + 3 * (r - 2), r == 3 ? to_string(roadMap[3][4]) : "\\");

    cout << "\n=============================== CITY MAP ===============================\n\n";
    for (size_t r = 0; r < grid.size(); r++) {
        size_t last = grid[r].find_last_not_of(' ');         
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

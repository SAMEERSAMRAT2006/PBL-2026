// citymap.h
// DSA: Weighted graph (adjacency list), Dijkstra, BFS, DFS, stack, queue.
// OOPS: Encapsulation, exception handling, const member functions.
#ifndef CITYMAP_H
#define CITYMAP_H

#include <string>
#include <vector>

const int NUM_LOCATIONS = 6;

struct Edge {
    int to;
    int km;
};

struct Route {
    std::vector<int> nodes;   // locations in travel order
    int km;                   // total distance, -1 if no route

    Route() : km(-1) {}
};

// How many people are usually at a place and their age mix (in percent).
// Three age groups: Child, Adult (middle) and Old.
struct PlaceProfile {
    int people;
    int childPct;
    int adultPct;
    int oldPct;
};

class CityMap {
private:
    std::string names[NUM_LOCATIONS];
    PlaceProfile profiles[NUM_LOCATIONS];
    std::vector<Edge> adj[NUM_LOCATIONS];   // adjacency list

    void addRoad(int a, int b, int km);
    void checkId(int id) const;             // throws out_of_range
    void dfsVisit(int u, bool visited[], std::vector<int> &order) const;

public:
    CityMap();

    std::string name(int id) const;
    int roadKm(int a, int b) const;         // 0 when there is no direct road

    PlaceProfile profile(int id) const;
    std::string describePlace(int id) const;

    // Splits n affected people into the three age groups using the
    // place's age mix (largest remainder method, so the parts add up to n).
    void splitPeople(int id, int n, int &children, int &adults,
                     int &elders) const;

    Route shortestRoute(int from, int to) const;     // Dijkstra (least km)
    Route fewestRoadsRoute(int from, int to) const;  // BFS (least roads)
    std::vector<int> dfsOrder(int start) const;      // DFS visit order
    bool isConnected() const;                        // DFS connectivity check

    void display() const;                   // list of roads
    void drawMap() const;                   // ASCII picture of the city
};

#endif

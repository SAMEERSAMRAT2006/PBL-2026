
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
    std::vector<int> nodes;  
    int km;                   

    Route() : km(-1) {}
};


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
    std::vector<Edge> adj[NUM_LOCATIONS];   
    void addRoad(int a, int b, int km);
    void checkId(int id) const;            
    void dfsVisit(int u, bool visited[], std::vector<int> &order) const;

public:
    CityMap();

    std::string name(int id) const;
    int roadKm(int a, int b) const;         
    PlaceProfile profile(int id) const;
    std::string describePlace(int id) const;

  
    void splitPeople(int id, int n, int &children, int &adults,
                     int &elders) const;

    Route shortestRoute(int from, int to) const;     
    Route fewestRoadsRoute(int from, int to) const; 
    std::vector<int> dfsOrder(int start) const;      
    bool isConnected() const;                        

    void display() const;                  
    void drawMap() const;                   
};

#endif

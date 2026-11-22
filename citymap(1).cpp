
#include "citymap.h"

#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <climits>
#include <functional>

using namespace std;

CityMap::CityMap() {
    const char *locationNames[NUM_LOCATIONS] = {
        "Hospital", "Fire Station", "Police Station",
        "Market", "School", "Highway Junction"
    };

    for (int i = 0; i < NUM_LOCATIONS; i++)
        names[i] = locationNames[i];

    // people, child %, adult %, old %
    const PlaceProfile data[NUM_LOCATIONS] = {
        {120, 15, 45, 40},   // Hospital: many old patients
        { 25,  0, 90, 10},   // Fire Station: mostly adults
        { 30,  5, 90,  5},   // Police Station: mostly adults
        {200, 15, 65, 20},   // Market: mixed crowd
        {400, 80, 18,  2},   // School: mostly children
        { 80, 10, 75, 15}    // Highway Junction: mostly adult travellers
    };

    for (int i = 0; i < NUM_LOCATIONS; i++)
        profiles[i] = data[i];

    addRoad(0, 1, 4);
    addRoad(0, 2, 2);
    addRoad(1, 3, 5);
    addRoad(2, 3, 8);
    addRoad(2, 4, 10);
    addRoad(3, 4, 2);
    addRoad(3, 5, 6);
    addRoad(4, 5, 3);
}

void CityMap::addRoad(int a, int b, int km) {
    adj[a].push_back({b, km});
    adj[b].push_back({a, km});
}

void CityMap::checkId(int id) const {
    if (id < 0 || id >= NUM_LOCATIONS)
        throw out_of_range("Invalid location id: " + to_string(id));
}

string CityMap::name(int id) const {
    checkId(id);
    return names[id];
}

PlaceProfile CityMap::profile(int id) const {
    checkId(id);
    return profiles[id];
}

string CityMap::describePlace(int id) const {
    checkId(id);

    const PlaceProfile &p = profiles[id];

    return names[id] + " (" + to_string(p.people) + " people: "
         + to_string(p.childPct) + "% child, "
         + to_string(p.adultPct) + "% adult, "
         + to_string(p.oldPct) + "% old)";
}

void CityMap::splitPeople(int id, int n, int &children, int &adults,
                          int &elders) const {
    checkId(id);

    const PlaceProfile &p = profiles[id];

    double exact[3] = {
        n * p.childPct / 100.0,
        n * p.adultPct / 100.0,
        n * p.oldPct / 100.0
    };

    int part[3];
    int used = 0;

    for (int i = 0; i < 3; i++) {
        part[i] = static_cast<int>(exact[i]);
        used += part[i];
    }

    
    int left = n - used;

    while (left > 0) {
        int best = 0;

        for (int i = 1; i < 3; i++)
            if (exact[i] - part[i] > exact[best] - part[best])
                best = i;

        part[best]++;
        left--;
    }

    children = part[0];
    adults = part[1];
    elders = part[2];
}

int CityMap::roadKm(int a, int b) const {
    checkId(a);
    checkId(b);

    for (const Edge &e : adj[a])
        if (e.to == b)
            return e.km;

    return 0;
}


static vector<int> buildPath(const int parent[], int to) {
    stack<int> st;

    for (int v = to; v != -1; v = parent[v])
        st.push(v);

    vector<int> path;

    while (!st.empty()) {
        path.push_back(st.top());
        st.pop();
    }

    return path;
}

// Dijkstra's algorithm with a min-heap (priority queue).
Route CityMap::shortestRoute(int from, int to) const {
    checkId(from);
    checkId(to);

    int dist[NUM_LOCATIONS];
    int parent[NUM_LOCATIONS];

    for (int i = 0; i < NUM_LOCATIONS; i++) {
        dist[i] = INT_MAX;
        parent[i] = -1;
    }

    priority_queue<pair<int, int>,
                   vector<pair<int, int> >,
                   greater<pair<int, int> > > pq;

    dist[from] = 0;
    pq.push(make_pair(0, from));

    while (!pq.empty()) {
        pair<int, int> top = pq.top();
        pq.pop();

        int d = top.first;
        int u = top.second;

        if (d > dist[u])
            continue;  

        for (const Edge &e : adj[u]) {
            if (dist[u] + e.km < dist[e.to]) {
                dist[e.to] = dist[u] + e.km;
                parent[e.to] = u;
                pq.push(make_pair(dist[e.to], e.to));
            }
        }
    }

    Route route;

    if (dist[to] == INT_MAX)
        return route;

    route.nodes = buildPath(parent, to);
    route.km = dist[to];
    return route;
}

// Breadth-first search with a queue: fewest roads, not fewest km.
Route CityMap::fewestRoadsRoute(int from, int to) const {
    checkId(from);
    checkId(to);

    bool visited[NUM_LOCATIONS] = {false};
    int parent[NUM_LOCATIONS];

    for (int i = 0; i < NUM_LOCATIONS; i++)
        parent[i] = -1;

    queue<int> q;
    visited[from] = true;
    q.push(from);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const Edge &e : adj[u]) {
            if (!visited[e.to]) {
                visited[e.to] = true;
                parent[e.to] = u;
                q.push(e.to);
            }
        }
    }

    Route route;

    if (!visited[to])
        return route;

    route.nodes = buildPath(parent, to);

    int km = 0;

    for (size_t i = 1; i < route.nodes.size(); i++)
        km += roadKm(route.nodes[i - 1], route.nodes[i]);

    route.km = km;
    return route;
}

// Depth-first search (recursive).
void CityMap::dfsVisit(int u, bool visited[], vector<int> &order) const {
    visited[u] = true;
    order.push_back(u);

    for (const Edge &e : adj[u])
        if (!visited[e.to])
            dfsVisit(e.to, visited, order);
}

vector<int> CityMap::dfsOrder(int start) const {
    checkId(start);

    bool visited[NUM_LOCATIONS] = {false};
    vector<int> order;

    dfsVisit(start, visited, order);
    return order;
}


bool CityMap::isConnected() const {
    return static_cast<int>(dfsOrder(0).size()) == NUM_LOCATIONS;
}

void CityMap::display() const {
    cout << "\nRoads (km):\n";

    for (int i = 0; i < NUM_LOCATIONS; i++)
        for (const Edge &e : adj[i])
            if (i < e.to)
                cout << "  " << names[i] << " <-> "
                     << names[e.to] << " : " << e.km << " km\n";
}

void CityMap::drawMap() const {
    vector<string> art;

    art.push_back("  [0 Hospital] -----4----- [1 Fire Station]");
    art.push_back(string(8, ' ') + "|" + string(25, ' ') + "|");
    art.push_back(string(8, ' ') + "2" + string(25, ' ') + "5");
    art.push_back(string(8, ' ') + "|" + string(25, ' ') + "|");
    art.push_back("  [2 Police Stn] -----8----- [3 Market] -----6----- "
                  "[5 Highway Jn]");
    art.push_back(string(8, ' ') + "|" + string(25, ' ') + "|" +
                  string(23, ' ') + "|");
    art.push_back(string(8, ' ') + "10" + string(24, ' ') + "2" +
                  string(23, ' ') + "3");
    art.push_back(string(8, ' ') + "|" + string(25, ' ') + "|" +
                  string(23, ' ') + "|");
    art.push_back(string(8, ' ') + "+" + string(19, '-') + " [4 School] " +
                  string(18, '-') + "+");

    cout << "\n";

    for (const string &line : art)
        cout << line << "\n";
}

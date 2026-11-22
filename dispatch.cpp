
#include "dispatch.h"

#include <iostream>
#include <iomanip>
#include <climits>

using namespace std;


RescueTeam::RescueTeam(const string &n, EmergencyType t, int base)
    : name(n), type(t), baseLocation(base), location(base),
      available(true) {}

RescueTeam::~RescueTeam() {}

void RescueTeam::resetToBase() {
    location = baseLocation;
    available = true;
}

Ambulance::Ambulance(const string &n, int base)
    : RescueTeam(n, MEDICAL, base) {}
int Ambulance::serviceMinutes() const { return 10; }
string Ambulance::vehicle() const { return "Ambulance"; }
string Ambulance::action() const { return "treating the patients"; }

FireTruck::FireTruck(const string &n, int base)
    : RescueTeam(n, FIRE, base) {}
int FireTruck::serviceMinutes() const { return 15; }
string FireTruck::vehicle() const { return "Fire truck"; }
string FireTruck::action() const { return "putting out the fire"; }

PoliceCar::PoliceCar(const string &n, int base)
    : RescueTeam(n, CRIME, base) {}
int PoliceCar::serviceMinutes() const { return 8; }
string PoliceCar::vehicle() const { return "Police car"; }
string PoliceCar::action() const { return "securing the area"; }



Dispatcher::Dispatcher(const CityMap &cityMap) : map(cityMap) {
   
    teams.push_back(new Ambulance("Ambulance-1", 0));
    teams.push_back(new Ambulance("Ambulance-2", 4));
    teams.push_back(new Ambulance("Ambulance-3", 5));
    teams.push_back(new FireTruck("FireTruck-1", 1));
    teams.push_back(new FireTruck("FireTruck-2", 5));
    teams.push_back(new FireTruck("FireTruck-3", 3));
    teams.push_back(new PoliceCar("PoliceCar-1", 2));
    teams.push_back(new PoliceCar("PoliceCar-2", 3));
    teams.push_back(new PoliceCar("PoliceCar-3", 4));
}

Dispatcher::~Dispatcher() {
    for (RescueTeam *t : teams)
        delete t;  
}

int Dispatcher::assign(EmergencyType teamType, int location, Route &route) {
    lock_guard<mutex> lock(m);

    int best = -1;
    int bestKm = INT_MAX;
    Route bestRoute;

    for (size_t i = 0; i < teams.size(); i++) {
        if (!teams[i]->isAvailable() || teams[i]->getType() != teamType)
            continue;

        Route r = map.shortestRoute(teams[i]->getLocation(), location);

        if (r.km >= 0 && r.km < bestKm) {
            bestKm = r.km;
            best = static_cast<int>(i);
            bestRoute = r;
        }
    }

    if (best == -1)
        return -1;

    teams[best]->setAvailable(false);
    route = bestRoute;
    return best;
}

void Dispatcher::release(int index, int newLocation) {
    lock_guard<mutex> lock(m);

    if (index < 0 || index >= static_cast<int>(teams.size()))
        return;

    teams[index]->setLocation(newLocation);
    teams[index]->setAvailable(true);
}

void Dispatcher::reset() {
    lock_guard<mutex> lock(m);

    for (RescueTeam *t : teams)
        t->resetToBase();
}

void Dispatcher::show() const {
    lock_guard<mutex> lock(m);

    cout << "\n========== RESCUE TEAMS ==========\n";

    for (const RescueTeam *t : teams) {
        cout << left << setw(13) << t->getName()
             << setw(12) << t->vehicle()
             << "Base: " << setw(17) << map.name(t->getBase())
             << "Now at: " << setw(17) << map.name(t->getLocation())
             << (t->isAvailable() ? "AVAILABLE" : "BUSY") << "\n";
    }
}

string Dispatcher::teamName(int index) const {
    lock_guard<mutex> lock(m);
    return teams.at(index)->getName();
}

int Dispatcher::serviceMinutes(int index) const {
    lock_guard<mutex> lock(m);
    return teams.at(index)->serviceMinutes();   
}

string Dispatcher::action(int index) const {
    lock_guard<mutex> lock(m);
    return teams.at(index)->action();           
}

// dispatch.cpp
// Finds the nearest available team of the correct type.
#include "dispatch.h"
#include "citymap.h"

#include <iostream>
#include <mutex>

using namespace std;

static RescueTeam teams[NUM_TEAMS];
static mutex teamMutex;

void initTeams() {
    lock_guard<mutex> lock(teamMutex);

    teams[0] = {"Ambulance-1", MEDICAL, 0, true};
    teams[1] = {"Ambulance-2", MEDICAL, 4, true};
    teams[2] = {"FireTruck-1", FIRE, 1, true};
    teams[3] = {"FireTruck-2", FIRE, 5, true};
    teams[4] = {"PoliceCar-1", CRIME, 2, true};
    teams[5] = {"PoliceCar-2", CRIME, 3, true};
}

int dispatchRescueTeam(const Emergency &emergency) {
    lock_guard<mutex> lock(teamMutex);

    int best = -1;
    int bestDistance = 1000000;

    for (int i = 0; i < NUM_TEAMS; i++) {
        if (teams[i].available && teams[i].type == emergency.type) {
            int distance = getDistance(teams[i].location, emergency.location);

            if (distance < bestDistance) {
                bestDistance = distance;
                best = i;
            }
        }
    }

    if (best == -1)
        return -1;

    teams[best].available = false;

    cout << "Dispatched " << teams[best].name
         << " | " << bestDistance << " km away\n";

    return best;
}

void showTeams() {
    lock_guard<mutex> lock(teamMutex);

    cout << "\n========== RESCUE TEAMS ==========\n";

    for (int i = 0; i < NUM_TEAMS; i++) {
        cout << teams[i].name
             << " | Base: " << getLocationName(teams[i].location)
             << " | Type: " << typeName(teams[i].type)
             << " | " << (teams[i].available ? "AVAILABLE" : "BUSY")
             << "\n";
    }
}

RescueTeam getTeam(int index) {
    lock_guard<mutex> lock(teamMutex);
    if (index < 0 || index >= NUM_TEAMS)
        return {"Unknown", MEDICAL, 0, false};
    return teams[index];
}

void releaseTeam(int index, int newLocation) {
    lock_guard<mutex> lock(teamMutex);

    if (index < 0 || index >= NUM_TEAMS)
        return;

    teams[index].location = newLocation;
    teams[index].available = true;
}

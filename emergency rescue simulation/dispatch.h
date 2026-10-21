// dispatch.h
#ifndef DISPATCH_H
#define DISPATCH_H

#include <string>
#include "emergency.h"

const int NUM_TEAMS = 6;

struct RescueTeam {
    std::string name;
    EmergencyType type;
    int location;
    bool available;
};

void initTeams();
int dispatchRescueTeam(const Emergency &emergency);
void showTeams();
RescueTeam getTeam(int index);
void releaseTeam(int index, int newLocation);

#endif

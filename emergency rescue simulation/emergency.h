// emergency.h
// Simple emergency management using a priority queue.
// Higher severity emergencies are handled first.
#ifndef EMERGENCY_H
#define EMERGENCY_H

#include <string>

enum EmergencyType { MEDICAL = 1, FIRE = 2, CRIME = 3 };

struct Emergency {
    int id;
    int location;
    EmergencyType type;
    int severity;
};

std::string typeName(EmergencyType type);

void addEmergency(int location, EmergencyType type, int severity);
bool serveNextEmergency(Emergency &emergency);
void putBackEmergency(const Emergency &emergency);
int pendingEmergencies();

#endif

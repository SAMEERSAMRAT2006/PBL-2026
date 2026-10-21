// emergency.cpp
#include "emergency.h"
#include "citymap.h"
#include "progress.h"

#include <iostream>
#include <queue>
#include <vector>
#include <mutex>

using namespace std;

struct CompareSeverity {
    bool operator()(const Emergency &a, const Emergency &b) const {
        if (a.severity != b.severity)
            return a.severity < b.severity;   // higher severity first
        return a.id > b.id;                   // older emergency first
    }
};

static priority_queue<Emergency, vector<Emergency>, CompareSeverity> emergencyQueue;
static int nextId = 1;
static mutex queueMutex;

string typeName(EmergencyType type) {
    if (type == MEDICAL) return "Medical";
    if (type == FIRE) return "Fire";
    if (type == CRIME) return "Crime";
    return "Unknown";
}

void addEmergency(int location, EmergencyType type, int severity) {
    Emergency e{nextId++, location, type, severity};

    {
        lock_guard<mutex> lock(queueMutex);
        emergencyQueue.push(e);
    }

    registerEmergency(e);

    cout << "\nEmergency #" << e.id << " reported: "
         << typeName(type) << " at "
         << getLocationName(location)
         << " | Severity: " << severity << "\n";
}

bool serveNextEmergency(Emergency &emergency) {
    lock_guard<mutex> lock(queueMutex);

    if (emergencyQueue.empty())
        return false;

    emergency = emergencyQueue.top();
    emergencyQueue.pop();
    return true;
}

void putBackEmergency(const Emergency &emergency) {
    lock_guard<mutex> lock(queueMutex);
    emergencyQueue.push(emergency);
}

int pendingEmergencies() {
    lock_guard<mutex> lock(queueMutex);
    return static_cast<int>(emergencyQueue.size());
}

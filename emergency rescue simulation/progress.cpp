// progress.cpp
// Stores a simple history of all emergencies and their current status.
#include "progress.h"
#include "citymap.h"
#include "dispatch.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <mutex>

using namespace std;

static vector<EmergencyProgress> records;
static mutex progressMutex;

long long currentTimeSeconds() {
    return chrono::duration_cast<chrono::seconds>(
        chrono::system_clock::now().time_since_epoch()).count();
}

static string formatTime(long long timestamp) {
    if (timestamp <= 0) return "--:--:--";

    time_t raw = static_cast<time_t>(timestamp);
    tm *local = localtime(&raw);

    if (!local) return "--:--:--";

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
    return string(buffer);
}

static string elapsedText(long long start, long long end = 0) {
    if (start <= 0) return "0s";

    long long now = end > 0 ? end : currentTimeSeconds();
    long long seconds = now - start;

    if (seconds < 0) seconds = 0;

    long long minutes = seconds / 60;
    long long secs = seconds % 60;

    if (minutes > 0)
        return to_string(minutes) + "m " + to_string(secs) + "s";

    return to_string(secs) + "s";
}

string progressStatusName(ProgressStatus status) {
    switch (status) {
        case REPORTED:   return "REPORTED";
        case WAITING:    return "WAITING";
        case DISPATCHED: return "DISPATCHED";
        case EN_ROUTE:   return "EN ROUTE";
        case ARRIVED:    return "ARRIVED";
        case COMPLETED:  return "COMPLETED";
    }

    return "UNKNOWN";
}

static int findRecord(int id) {
    for (int i = 0; i < static_cast<int>(records.size()); i++) {
        if (records[i].emergencyId == id)
            return i;
    }

    return -1;
}

void initProgress() {
    lock_guard<mutex> lock(progressMutex);
    records.clear();
}

void registerEmergency(const Emergency &emergency) {
    lock_guard<mutex> lock(progressMutex);

    EmergencyProgress record{
        emergency.id,
        emergency.location,
        emergency.type,
        emergency.severity,
        REPORTED,
        -1,
        currentTimeSeconds(),
        0,
        0
    };

    records.push_back(record);
}

void markWaiting(const Emergency &emergency) {
    lock_guard<mutex> lock(progressMutex);

    int index = findRecord(emergency.id);
    if (index >= 0)
        records[index].status = WAITING;
}

void markDispatched(const Emergency &emergency, int teamIndex) {
    lock_guard<mutex> lock(progressMutex);

    int index = findRecord(emergency.id);
    if (index >= 0) {
        records[index].status = DISPATCHED;
        records[index].teamIndex = teamIndex;
        records[index].startedAt = currentTimeSeconds();
    }
}

void markEnRoute(const Emergency &emergency, int teamIndex) {
    lock_guard<mutex> lock(progressMutex);

    int index = findRecord(emergency.id);
    if (index >= 0) {
        records[index].status = EN_ROUTE;
        records[index].teamIndex = teamIndex;
    }
}

void markArrived(const Emergency &emergency, int teamIndex) {
    lock_guard<mutex> lock(progressMutex);

    int index = findRecord(emergency.id);
    if (index >= 0) {
        records[index].status = ARRIVED;
        records[index].teamIndex = teamIndex;
    }
}

void markCompleted(const Emergency &emergency, int teamIndex) {
    lock_guard<mutex> lock(progressMutex);

    int index = findRecord(emergency.id);
    if (index >= 0) {
        records[index].status = COMPLETED;
        records[index].teamIndex = teamIndex;
        records[index].finishedAt = currentTimeSeconds();
    }
}

void showProgress() {
    lock_guard<mutex> lock(progressMutex);

    cout << "\n========== CURRENT EMERGENCY REPORT ==========\n";

    if (records.empty()) {
        cout << "No emergencies have been reported.\n";
        return;
    }

    long long now = currentTimeSeconds();

    for (const EmergencyProgress &record : records) {
        cout << "\nEmergency #" << record.emergencyId
             << " | " << typeName(record.type)
             << " | Severity: " << record.severity << "\n";

        cout << "Location : " << getLocationName(record.location) << "\n";
        cout << "Status   : " << progressStatusName(record.status) << "\n";

        if (record.teamIndex >= 0)
            cout << "Rescuer  : " << getTeam(record.teamIndex).name << "\n";
        else
            cout << "Rescuer  : Not assigned\n";

        cout << "Reported : " << formatTime(record.reportedAt) << "\n";

        if (record.startedAt > 0)
            cout << "Started  : " << formatTime(record.startedAt) << "\n";

        if (record.finishedAt > 0) {
            cout << "Finished : " << formatTime(record.finishedAt) << "\n";
            cout << "Total time: "
                 << elapsedText(record.reportedAt, record.finishedAt) << "\n";
        } else {
            cout << "Time passed: "
                 << elapsedText(record.reportedAt, now) << "\n";
        }
    }

    cout << "\n==============================================\n";
}


#include "ers.h"

#include <iostream>
#include <queue>
#include <vector>
#include <utility>

using namespace std;

static vector<Emergency> history;                
static priority_queue<pair<int, int> > waitingQueue;

string typeName(EmergencyType type) {
    if (type == MEDICAL) return "Medical";
    if (type == FIRE) return "Fire";
    return "Crime";
}

string statusName(Status status) {
    switch (status) {
        case REPORTED:  return "REPORTED";
        case WAITING:   return "WAITING (no free team)";
        case EN_ROUTE:  return "EN ROUTE";
        case ON_SCENE:  return "ON SCENE";
        default:        return "COMPLETED";
    }
}

// Function 3: record a new emergency and put it in the priority queue.
int addEmergency(int location, EmergencyType type, int severity) {
    lock_guard<recursive_mutex> lock(ersLock);

    int id = static_cast<int>(history.size()) + 1;
    Emergency e = {id, location, type, severity, REPORTED, -1, simNow(), 0, 0, 0};

    history.push_back(e);
    waitingQueue.push(make_pair(severity, -id));

    liveLog(e.reportedAt, "NEW  Emergency #" + to_string(id) + " reported: " + typeName(type) +
            " at " + getLocationName(location) + " | severity " + to_string(severity) + "/5");
    return id;
}

// Function 4: take the most urgent emergency out of the queue.
bool serveNextEmergency(int &id) {
    lock_guard<recursive_mutex> lock(ersLock);

    if (waitingQueue.empty())
        return false;

    id = -waitingQueue.top().second;
    waitingQueue.pop();
    return true;
}

// used when no team was free: the emergency goes back into the queue
void putBackEmergency(int id) {
    lock_guard<recursive_mutex> lock(ersLock);
    waitingQueue.push(make_pair(history[id - 1].severity, -id));
}

Emergency &getEmergency(int id) {
    return history[id - 1];
}

// completed = true  -> how many are finished
// completed = false -> how many are still open
int countEmergencies(bool completed) {
    lock_guard<recursive_mutex> lock(ersLock);

    int count = 0;
    for (size_t i = 0; i < history.size(); i++)
        if ((history[i].status == COMPLETED) == completed)
            count++;
    return count;
}

// "!1 !3" = emergencies #1 and #3 are still open at this place (used by the map)
string openEmergencyMarks(int location) {
    lock_guard<recursive_mutex> lock(ersLock);

    string marks;
    for (size_t i = 0; i < history.size(); i++)
        if (history[i].location == location && history[i].status != COMPLETED)
            marks += "!" + to_string(history[i].id) + " ";
    return marks;
}

static string minutes(long long seconds) {
    return to_string(seconds / 60) + " min";
}

// Progress tracker: one block per emergency, plus the average response time.
void showReport() {
    lock_guard<recursive_mutex> lock(ersLock);

    cout << "\n=============== EMERGENCY REPORT (time " << clockText(simNow()) << ") ===============\n";

    if (history.empty()) {
        cout << "No emergencies have been reported yet.\n";
        return;
    }

    long long totalResponse = 0;
    int responded = 0;

    for (size_t i = 0; i < history.size(); i++) {
        const Emergency &e = history[i];

        cout << "\nEmergency #" << e.id << " | " << typeName(e.type)
             << " | Severity " << e.severity << "/5\n";
        cout << "  Location : " << getLocationName(e.location) << "\n";
        cout << "  Status   : " << statusName(e.status) << "\n";
        cout << "  Team     : " << (e.team >= 0 ? teamName(e.team) : "not assigned yet") << "\n";
        cout << "  Reported : " << clockText(e.reportedAt) << "\n";

        if (e.startedAt > 0)
            cout << "  Dispatch : " << clockText(e.startedAt) << "\n";

        if (e.arrivedAt > 0) {
            cout << "  Arrived  : " << clockText(e.arrivedAt)
                 << "  (response time " << minutes(e.arrivedAt - e.reportedAt) << ")\n";
            totalResponse += e.arrivedAt - e.reportedAt;
            responded++;
        }

        if (e.finishedAt > 0)
            cout << "  Finished : " << clockText(e.finishedAt)
                 << "  (total " << minutes(e.finishedAt - e.reportedAt) << ")\n";
        else
            cout << "  Open for : " << minutes(simNow() - e.reportedAt) << "\n";
    }

    cout << "\nCompleted: " << countEmergencies(true)
         << " | Still open: " << countEmergencies(false) << "\n";
    if (responded > 0)
        cout << "Average response time: " << minutes(totalResponse / responded) << "\n";
    cout << "=========================================================\n";
}

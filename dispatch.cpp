
#include "ers.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

using namespace std;

recursive_mutex ersLock;

static RescueTeam teams[NUM_TEAMS];
static atomic<bool> running(false);
static thread engineThread;
static chrono::steady_clock::time_point realStart;

// ---------------- simulated clock ----------------

long long simNow() {
    double realSeconds = chrono::duration<double>(chrono::steady_clock::now() - realStart).count();
    return SIM_START + static_cast<long long>(realSeconds * TIME_SPEED);
}

static string twoDigits(long long n) {
    return (n < 10 ? "0" : "") + to_string(n);
}

string clockText(long long t) {
    long long mins = (t / 60) % 1440;
    return twoDigits(mins / 60) + ":" + twoDigits(mins % 60);
}


void liveLog(long long when, const string &message) {
    lock_guard<recursive_mutex> lock(ersLock);
    ofstream out(LIVE_FILE, ios::app | ios::binary);
    out << "[" << clockText(when) << "] " << message << "\n";
}

// ---------------- teams ----------------

static void initTeams() {
    teams[0] = {"Ambulance-1", MEDICAL, 0, true, 0, 0, 0};
    teams[1] = {"Ambulance-2", MEDICAL, 4, true, 0, 0, 0};
    teams[2] = {"FireTruck-1", FIRE,    1, true, 0, 0, 0};
    teams[3] = {"FireTruck-2", FIRE,    5, true, 0, 0, 0};
    teams[4] = {"PoliceCar-1", CRIME,   2, true, 0, 0, 0};
    teams[5] = {"PoliceCar-2", CRIME,   3, true, 0, 0, 0};
}

string teamName(int index) {
    return (index >= 0 && index < NUM_TEAMS) ? teams[index].name : "Unknown";
}

static long long travelSeconds(int from, int to) {
    return getDistance(from, to) * MIN_PER_KM * 60LL;
}

static long long workSeconds(const Emergency &e) {
    return (10 + 4 * e.severity) * 60LL;      
}


static bool dispatchRescueTeam(int id) {
    Emergency &e = getEmergency(id);
    long long now = simNow();

    int best = -1;
    for (int i = 0; i < NUM_TEAMS; i++) {
        if (teams[i].available && teams[i].type == e.type) {
            if (best == -1 ||
                getDistance(teams[i].location, e.location) < getDistance(teams[best].location, e.location))
                best = i;
        }
    }

    if (best == -1) {
        if (e.status == REPORTED) {
            e.status = WAITING;
            liveLog(now, "WAIT Emergency #" + to_string(id) + " is queued: all " +
                    typeName(e.type) + " teams are busy");
        }
        return false;
    }

    RescueTeam &t = teams[best];
    string waited = (e.status == WAITING) ? " | waited " + to_string((now - e.reportedAt) / 60) + " min" : "";

    t.available = false;
    t.job = id;
    t.nextStop = getNextStop(t.location, e.location);     // first stop on the way
    t.nextAt = now + travelSeconds(t.location, t.nextStop);
    e.team = best;
    e.status = EN_ROUTE;
    e.startedAt = now;

    liveLog(now, "SEND " + t.name + " -> Emergency #" + to_string(id) + " | route: " +
            routeText(t.location, e.location) + " | " + to_string(getDistance(t.location, e.location)) +
            " km | ETA " + clockText(now + travelSeconds(t.location, e.location)) + waited);
    return true;
}


static void moveTeams() {
    long long now = simNow();

    for (int i = 0; i < NUM_TEAMS; i++) {
        RescueTeam &t = teams[i];
        if (t.available || now < t.nextAt)
            continue;

        Emergency &e = getEmergency(t.job);
        long long when = t.nextAt;
        string id = to_string(e.id);

        if (e.status == EN_ROUTE) {
            t.location = t.nextStop;                       

            if (t.location == e.location) {                
                e.status = ON_SCENE;
                e.arrivedAt = when;
                t.nextAt = when + workSeconds(e);
                liveLog(when, "ARRV " + t.name + " reached " + getLocationName(e.location) +
                        " (Emergency #" + id + ") | response time " +
                        to_string((when - e.reportedAt) / 60) + " min | working until " +
                        clockText(t.nextAt));
            } else {                                       // still on the way
                t.nextStop = getNextStop(t.location, e.location);
                t.nextAt = when + travelSeconds(t.location, t.nextStop);
                liveLog(when, "MOVE " + t.name + " passed " + getLocationName(t.location) +
                        ", heading to " + getLocationName(t.nextStop) + " (Emergency #" + id + ")");
            }
        } else {                                           
            e.status = COMPLETED;
            e.finishedAt = when;
            t.available = true;
            t.job = 0;
            liveLog(when, "DONE " + t.name + " finished Emergency #" + id + " | total " +
                    to_string((when - e.reportedAt) / 60) + " min | team is free at " +
                    getLocationName(t.location));
        }
    }
}

static void assignWaiting() {
    vector<int> notSent;
    int id;

    while (serveNextEmergency(id))
        if (!dispatchRescueTeam(id))
            notSent.push_back(id);

    for (size_t i = 0; i < notSent.size(); i++)
        putBackEmergency(notSent[i]);
}

static void engineLoop() {
    while (running) {
        {
            lock_guard<recursive_mutex> lock(ersLock);
            moveTeams();
            assignWaiting();
        }
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

// ---------------- start / stop ----------------

bool startSimulation() {
    if (running) {
        cout << "\nSimulation is already running.\n";
        return false;
    }

    loadCityMap();
    initTeams();
    { ofstream clear(LIVE_FILE, ios::trunc | ios::binary); }  

    realStart = chrono::steady_clock::now();
    running = true;
    liveLog(SIM_START, "SYSTEM Simulation started - city is active, 6 rescue teams ready");
    engineThread = thread(engineLoop);

    cout << "\n============================================\n";
    cout << "       EMERGENCY SIMULATION STARTED\n";
    cout << "============================================\n";
    cout << "City clock started at " << clockText(SIM_START) << " (1 real second = 1 minute).\n";
    cout << "You can now report emergencies.\n";
    return true;
}

void stopSimulation() {
    if (!running)
        return;

    running = false;
    if (engineThread.joinable())
        engineThread.join();

    liveLog(simNow(), "SYSTEM SIMULATION STOPPED");
}

bool simulationRunning() {
    return running;
}

// ---------------- screens ----------------

void showTeams() {
    lock_guard<recursive_mutex> lock(ersLock);

    cout << "\n=================== RESCUE TEAMS (time " << clockText(simNow()) << ") ===================\n";
    for (int i = 0; i < NUM_TEAMS; i++) {
        const RescueTeam &t = teams[i];
        cout << t.name << " | " << typeName(t.type) << " | at " << getLocationName(t.location) << " | ";
        if (t.available)
            cout << "AVAILABLE\n";
        else
            cout << "BUSY - Emergency #" << t.job << " (" << statusName(getEmergency(t.job).status) << ")\n";
    }
    cout << "=========================================================\n";
}


void showLiveMap() {
    lock_guard<recursive_mutex> lock(ersLock);

    string marks[NUM_LOCATIONS];
    for (int i = 0; i < NUM_TEAMS; i++)
        marks[teams[i].location] += string(1, teams[i].name[0]) + teams[i].name.back() +
                                    (teams[i].available ? "" : "*") + " ";
    for (int i = 0; i < NUM_LOCATIONS; i++)
        marks[i] += openEmergencyMarks(i);

    cout << "\nCity time: " << clockText(simNow()) << "\n";
    displayMap(marks);
}

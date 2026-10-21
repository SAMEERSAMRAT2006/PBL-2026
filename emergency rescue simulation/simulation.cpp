// simulation.cpp
// Runs rescue work in the background while the operator uses the menu.
#include "simulation.h"
#include "emergency.h"
#include "dispatch.h"
#include "citymap.h"
#include "progress.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <atomic>
#include <mutex>

using namespace std;

static atomic<bool> running(false);
static thread simulationThread;
static mutex outputMutex;

static string clockTime(long long timestamp) {
    time_t raw = static_cast<time_t>(timestamp);
    tm *local = localtime(&raw);

    if (!local)
        return "--:--:--";

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", local);

    return string(buffer);
}

static vector<int> buildRoute(int from, int to) {
    vector<int> route;
    route.push_back(from);

    // Phase II keeps routing simple. It uses an existing direct road.
    // Dijkstra shortest-path routing will be added in the next phase.
    if (getRoadDistance(from, to) != -1)
        route.push_back(to);

    return route;
}

static void rescueOne(Emergency e) {
    int teamIndex = dispatchRescueTeam(e);

    if (teamIndex == -1) {
        markWaiting(e);
        putBackEmergency(e);

        lock_guard<mutex> lock(outputMutex);
        cout << "\n[Simulation] Emergency #" << e.id
             << " is WAITING because no suitable team is free.\n";
        return;
    }

    RescueTeam team = getTeam(teamIndex);

    markDispatched(e, teamIndex);

    {
        lock_guard<mutex> lock(outputMutex);

        cout << "\n[ " << clockTime(currentTimeSeconds()) << " ] "
             << team.name << " started Emergency #"
             << e.id << "\n";

        cout << "Route: ";
    }

    vector<int> route = buildRoute(team.location, e.location);

    {
        lock_guard<mutex> lock(outputMutex);

        for (size_t i = 0; i < route.size(); i++) {
            cout << getLocationName(route[i]);

            if (i + 1 < route.size())
                cout << " -> ";
        }

        cout << "\n";
    }

    markEnRoute(e, teamIndex);

    for (size_t i = 1; i < route.size(); i++) {
        int from = route[i - 1];
        int to = route[i];

        {
            lock_guard<mutex> lock(outputMutex);

            cout << "[ " << clockTime(currentTimeSeconds()) << " ] "
                 << team.name << " travelling: "
                 << getLocationName(from) << " -> "
                 << getLocationName(to)
                 << " (" << getRoadDistance(from, to)
                 << " km)\n";
        }

        // One second represents part of the simulated journey.
        this_thread::sleep_for(chrono::seconds(1));
    }

    markArrived(e, teamIndex);

    {
        lock_guard<mutex> lock(outputMutex);

        cout << "[ " << clockTime(currentTimeSeconds()) << " ] "
             << team.name << " ARRIVED at "
             << getLocationName(e.location) << "\n";
    }

    this_thread::sleep_for(chrono::seconds(1));

    markCompleted(e, teamIndex);

    releaseTeam(teamIndex, e.location);

    {
        lock_guard<mutex> lock(outputMutex);

        cout << "[ " << clockTime(currentTimeSeconds()) << " ] "
             << team.name << " FINISHED Emergency #"
             << e.id << "\n";
        cout << "Team is now available at "
             << getLocationName(e.location) << ".\n";
    }
}

static void simulationLoop() {
    while (running) {
        Emergency e;

        if (!serveNextEmergency(e)) {
            this_thread::sleep_for(chrono::milliseconds(300));
            continue;
        }

        rescueOne(e);
    }
}

void startSimulation() {
    if (running) {
        cout << "\nSimulation is already running in the background.\n";
        return;
    }

    loadCityMap();
    initTeams();
    initProgress();

    running = true;

    simulationThread = thread(simulationLoop);

    cout << "\n============================================\n";
    cout << "       EMERGENCY SIMULATION STARTED\n";
    cout << "============================================\n";
    cout << "City is active.\n";
    cout << "You can now report emergencies.\n";
    cout << "Rescue operations will run in the background.\n";
}

void stopSimulation() {
    if (!running)
        return;

    running = false;

    if (simulationThread.joinable())
        simulationThread.join();

    cout << "\nSimulation stopped.\n";
}

bool simulationRunning() {
    return running;
}

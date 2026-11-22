// simulation.cpp
#include "simulation.h"

#include <iostream>
#include <cstdlib>

using namespace std;

static const int MINUTES_PER_KM = 2;   // travel speed in simulated time
static const char *LIVE_FILE = "live_log.txt";
static const char *REPORT_FILE = "rescue_report.txt";

// ---------------- SimClock ----------------

SimClock::SimClock() : startTime(chrono::steady_clock::now()) {}

void SimClock::restart() {
    startTime = chrono::steady_clock::now();
}

int SimClock::nowMinutes() const {
    double seconds = chrono::duration<double>(
        chrono::steady_clock::now() - startTime).count();

    return START_MINUTE + static_cast<int>(seconds * MINUTES_PER_REAL_SECOND);
}

// ---------------- Simulation ----------------

Simulation::Simulation()
    : map(),
      queue(),
      dispatcher(map),
      tracker(&map, LIVE_FILE),
      clock(),
      running(false) {}

Simulation::~Simulation() {
    stop();
}

string Simulation::routeText(const Route &route) const {
    string text;

    for (size_t i = 0; i < route.nodes.size(); i++) {
        text += map.name(route.nodes[i]);

        if (i + 1 < route.nodes.size())
            text += " -> ";
    }

    return text;
}

bool Simulation::sleepSim(int simMinutes) {
    int remainingMs =
        simMinutes * 1000 / SimClock::MINUTES_PER_REAL_SECOND;

    while (remainingMs > 0) {
        if (!running)
            return false;

        int slice = remainingMs < 50 ? remainingMs : 50;
        this_thread::sleep_for(chrono::milliseconds(slice));
        remainingMs -= slice;
    }

    return running;
}

// Runs in its own thread, so several teams can work at the same time.
void Simulation::runRescue(Emergency e, int teamIndex, Route route) {
    string team = dispatcher.teamName(teamIndex);
    string where = map.name(e.getLocation());
    string id = to_string(e.getId());

    tracker.setStatus(e.getId(), DISPATCHED, clock.nowMinutes());

    tracker.log(clock.nowMinutes(),
        team + " dispatched to Emergency #" + id + " ("
        + Emergency::typeName(e.getType()) + ", severity "
        + to_string(e.getSeverity()) + ") at " + where);

    tracker.log(clock.nowMinutes(),
        team + " route: " + routeText(route) + " ("
        + to_string(route.km) + " km)");

    tracker.setStatus(e.getId(), EN_ROUTE, clock.nowMinutes());

    if (route.nodes.size() == 1)
        tracker.log(clock.nowMinutes(),
            team + " is already at " + where);

    for (size_t i = 1; i < route.nodes.size(); i++) {
        int from = route.nodes[i - 1];
        int to = route.nodes[i];
        int km = map.roadKm(from, to);

        tracker.log(clock.nowMinutes(),
            team + " travelling " + map.name(from) + " -> "
            + map.name(to) + " (" + to_string(km) + " km)");

        if (!sleepSim(km * MINUTES_PER_KM)) {
            dispatcher.release(teamIndex, from);
            return;
        }
    }

    tracker.setStatus(e.getId(), ARRIVED, clock.nowMinutes());

    tracker.log(clock.nowMinutes(),
        team + " ARRIVED at " + where + ", "
        + dispatcher.action(teamIndex));

    if (!sleepSim(dispatcher.serviceMinutes(teamIndex))) {
        dispatcher.release(teamIndex, e.getLocation());
        return;
    }

    int finish = clock.nowMinutes();

    bool allDone = tracker.teamFinished(e.getId(), finish);
    dispatcher.release(teamIndex, e.getLocation());

    tracker.log(finish,
        team + " FINISHED its work at Emergency #" + id
        + ". Team is free at " + where);

    if (allDone)
        tracker.log(finish,
            "Emergency #" + id + " RESOLVED: all "
            + to_string(e.needTotal()) + " team(s) finished (total "
            + to_string(finish - e.getReportedAt()) + " min)");
}

// Dispatch loop: takes all pending emergencies in priority order.
// Each emergency may need several teams of different types. Every free
// team that is needed is sent (nearest first). If some teams are still
// missing, the emergency goes back in the queue to get them later, so one
// blocked emergency never stops the others.
void Simulation::loop() {
    const EmergencyType teamTypes[3] = {MEDICAL, FIRE, CRIME};

    while (running) {
        vector<Emergency> batch = queue.drainAll();

        for (Emergency &e : batch) {
            if (!running) {
                queue.push(e);
                continue;
            }

            for (EmergencyType t : teamTypes) {
                while (e.getRemaining(t) > 0) {
                    Route route;
                    int teamIndex = dispatcher.assign(
                        t, e.getLocation(), route);

                    if (teamIndex == -1)
                        break;   // no free team of this type right now

                    e.consume(t);
                    tracker.teamSent(e.getId(),
                                     dispatcher.teamName(teamIndex));

                    workers.emplace_back(&Simulation::runRescue,
                                         this, e, teamIndex, route);
                }
            }

            if (e.remainingTotal() > 0) {
                tracker.setStatus(e.getId(), WAITING, clock.nowMinutes());

                if (!e.waitNoted()) {
                    tracker.log(clock.nowMinutes(),
                        "Emergency #" + to_string(e.getId())
                        + " is WAITING for: " + e.remainingText());

                    e.setWaitNoted(true);
                }

                queue.push(e);
            }
        }

        this_thread::sleep_for(chrono::milliseconds(300));
    }
}

void Simulation::start() {
    if (running) {
        cout << "\nSimulation is already running.\n";
        return;
    }

    if (!map.isConnected()) {
        cout << "\nCity map check (DFS) failed: not all locations "
                "are connected.\n";
        return;
    }

    queue.clear();
    Emergency::resetCounter();
    dispatcher.reset();
    tracker.reset();
    clock.restart();

    running = true;

    tracker.log(clock.nowMinutes(),
        "Simulation started. DFS check: all locations are connected.");

    loopThread = thread(&Simulation::loop, this);

    cout << "\n============================================\n";
    cout << "       EMERGENCY SIMULATION STARTED\n";
    cout << "============================================\n";
    cout << "You can now report emergencies.\n";
    cout << "Rescue work runs in the background (1 real second = "
         << SimClock::MINUTES_PER_REAL_SECOND << " simulated minutes).\n";
    cout << "Live updates go to " << LIVE_FILE
         << " (menu option 7 opens them in a second window).\n";
}

void Simulation::stop() {
    if (!running)
        return;

    running = false;

    if (loopThread.joinable())
        loopThread.join();

    for (thread &t : workers)
        if (t.joinable())
            t.join();

    workers.clear();

    cout << "\nSimulation stopped.\n";
}

void Simulation::report(int location, EmergencyType type,
                        int intensity, int people) {
    if (!running) {
        cout << "\nStart the simulation first.\n";
        return;
    }

    int children, adults, elders;
    map.splitPeople(location, people, children, adults, elders);

    Emergency e(location, type, intensity,
                children, adults, elders, clock.nowMinutes());

    tracker.add(e);
    queue.push(e);

    tracker.log(clock.nowMinutes(),
        "Emergency #" + to_string(e.getId()) + " reported: "
        + Emergency::typeName(type) + " at " + map.name(location));

    tracker.log(clock.nowMinutes(),
        "Emergency #" + to_string(e.getId()) + " assessment: "
        + e.summary());

    cout << "\n" << e << " reported at " << map.name(location) << ".\n";
    cout << e.summary() << "\n";
}

void Simulation::showFormula() const {
    cout << Emergency::formulaText();
}

void Simulation::showReport() const {
    tracker.show();
}

void Simulation::showMap() const {
    cout << "\n========== CITY MAP ==========\n";

    for (int i = 0; i < NUM_LOCATIONS; i++)
        cout << i << " = " << map.describePlace(i) << "\n";

    map.drawMap();
    map.display();

    vector<int> order = map.dfsOrder(0);

    cout << "\nDFS visit order from " << map.name(0) << ": ";

    for (size_t i = 0; i < order.size(); i++)
        cout << map.name(order[i])
             << (i + 1 < order.size() ? " -> " : "\n");

    cout << "City connected: " << (map.isConnected() ? "YES" : "NO") << "\n";
}

void Simulation::showTeams() const {
    dispatcher.show();
}

void Simulation::routeFinder(int from, int to) const {
    Route shortest = map.shortestRoute(from, to);
    Route fewest = map.fewestRoadsRoute(from, to);

    cout << "\n========== ROUTE FINDER ==========\n";

    if (shortest.km < 0) {
        cout << "No route between these locations.\n";
        return;
    }

    cout << "Shortest distance (Dijkstra): " << routeText(shortest)
         << " | " << shortest.km << " km\n";

    cout << "Fewest roads (BFS)          : " << routeText(fewest)
         << " | " << fewest.km << " km, "
         << (fewest.nodes.size() - 1) << " road(s)\n";
}

void Simulation::saveReport() const {
    if (tracker.saveReport(REPORT_FILE))
        cout << "\nReport saved to " << REPORT_FILE << ".\n";
    else
        cout << "\nCould not write " << REPORT_FILE << ".\n";
}

void Simulation::openLiveWindow() const {
    if (!running) {
        cout << "\nStart the simulation first.\n";
        return;
    }

#ifdef _WIN32
    system("start powershell -NoExit -Command "
           "\"Get-Content live_log.txt -Wait\"");
    cout << "\nLive window opened.\n";
#else
    cout << "\nOpen a second terminal in this folder and run:\n"
         << "  tail -f " << LIVE_FILE << "\n";
#endif
}

int Simulation::pending() const {
    return queue.size();
}

int Simulation::active() const {
    return tracker.activeCount();
}

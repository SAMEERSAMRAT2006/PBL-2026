// simulation.h
// Sameer's part (2/3): the simulation engine.
//
// OOPS: Composition (Simulation owns the map, queue, dispatcher and tracker),
//       member initialiser list, destructor that stops threads,
//       private helpers, exception handling.
// DSA : Priority queue of emergencies, graph algorithms through CityMap.
#ifndef SIMULATION_H
#define SIMULATION_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "citymap.h"
#include "emergency.h"
#include "dispatch.h"

// Simulated clock: 1 real second = 5 simulated minutes, day starts at 08:00.
class SimClock {
private:
    std::chrono::steady_clock::time_point startTime;

public:
    enum { START_MINUTE = 8 * 60, MINUTES_PER_REAL_SECOND = 5 };

    SimClock();
    void restart();
    int nowMinutes() const;
};

class Simulation {
private:
    CityMap map;
    EmergencyQueue queue;
    Dispatcher dispatcher;
    ProgressTracker tracker;
    SimClock clock;

    std::atomic<bool> running;
    std::thread loopThread;
    std::vector<std::thread> workers;   // one thread per active rescue

    void loop();
    void runRescue(Emergency e, int teamIndex, Route route);
    bool sleepSim(int simMinutes);      // false if the simulation was stopped
    std::string routeText(const Route &route) const;

public:
    Simulation();
    ~Simulation();

    Simulation(const Simulation &) = delete;
    Simulation &operator=(const Simulation &) = delete;

    void start();
    void stop();
    bool isRunning() const { return running; }

    // intensity: 1 (minor) to 5 (catastrophic); people: how many are affected.
    // Severity is calculated by the formula, not typed in.
    void report(int location, EmergencyType type, int intensity, int people);

    void showFormula() const;
    void showReport() const;
    void showMap() const;
    void showTeams() const;
    void routeFinder(int from, int to) const;
    void saveReport() const;
    void openLiveWindow() const;

    int pending() const;
    int active() const;
};

#endif

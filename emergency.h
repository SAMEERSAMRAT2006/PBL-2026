// emergency.h
// Paras's part: emergencies, severity formula, priority queue, progress.
//
// DSA : Priority queue (max-heap), linked list (event log), vector.
// OOPS: Class, static members, operator< and operator<<, friend function,
//       encapsulation, const correctness, file handling.
#ifndef EMERGENCY_H
#define EMERGENCY_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <ostream>

#include "citymap.h"
#include "linkedlist.h"

enum EmergencyType { MEDICAL = 1, FIRE = 2, CRIME = 3 };

enum ProgressStatus {
    REPORTED, WAITING, DISPATCHED, EN_ROUTE, ARRIVED, COMPLETED
};

// Team types are indexed by (EmergencyType - 1):
// 0 = Ambulance, 1 = Fire truck, 2 = Police car.
class Emergency {
private:
    static int counter;      // shared by all objects: gives unique ids

    int id;
    int location;
    EmergencyType type;      // main kind of incident
    int intensity;           // 1 (minor) to 5 (catastrophic)
    int children;            // affected people, by age group
    int adults;
    int elders;
    double points;           // severity points from the formula
    int severity;            // 1 to 5, from the points
    int reportedAt;          // simulated minute of the day
    int need[3];             // teams required, per team type
    int remaining[3];        // teams still to be sent, per team type
    bool waitLogged;         // "waiting" already written to the log

    void planTeams(double weighted);

public:
    // Severity formula (weights for each age group):
    //   weighted = 1.5 x Child + 1.0 x Adult + 1.3 x Old
    //   points   = 10 x Intensity + 0.5 x weighted
    static constexpr double CHILD_WEIGHT = 1.5;
    static constexpr double ADULT_WEIGHT = 1.0;
    static constexpr double OLD_WEIGHT = 1.3;
    static constexpr double INTENSITY_FACTOR = 10.0;
    static constexpr double PEOPLE_FACTOR = 0.5;

    Emergency();
    Emergency(int location, EmergencyType type, int intensity,
              int children, int adults, int elders, int reportedAt);

    int getId() const { return id; }
    int getLocation() const { return location; }
    EmergencyType getType() const { return type; }
    int getIntensity() const { return intensity; }
    int getSeverity() const { return severity; }
    double getPoints() const { return points; }
    int getReportedAt() const { return reportedAt; }
    int getChildren() const { return children; }
    int getAdults() const { return adults; }
    int getElders() const { return elders; }
    int getPeople() const { return children + adults + elders; }

    int getNeed(EmergencyType teamType) const;
    int getRemaining(EmergencyType teamType) const;
    int needTotal() const;
    int remainingTotal() const;
    void consume(EmergencyType teamType);   // one more team has been sent

    bool waitNoted() const { return waitLogged; }
    void setWaitNoted(bool value) { waitLogged = value; }

    // Higher severity first; equal severity -> higher points first;
    // still equal -> the older emergency (smaller id) first.
    bool operator<(const Emergency &other) const;

    friend std::ostream &operator<<(std::ostream &out, const Emergency &e);

    std::string summary() const;       // people, points, severity, teams
    std::string remainingText() const; // e.g. "1 ambulance, 2 fire truck"
    std::string needsText() const;

    static std::string typeName(EmergencyType type);
    static std::string teamLabel(EmergencyType teamType);
    static std::string formulaText();
    static int levelFromPoints(double points);
    static void resetCounter();
};

// Thread-safe priority queue of pending emergencies.
class EmergencyQueue {
private:
    std::priority_queue<Emergency> heap;
    mutable std::mutex m;

public:
    void push(const Emergency &e);
    std::vector<Emergency> drainAll();   // removes all, highest priority first
    int size() const;
    void clear();
};

struct ProgressRecord {
    int id;
    int location;
    EmergencyType type;
    int intensity;
    int severity;
    double points;
    int children;
    int adults;
    int elders;
    int need[3];
    int needTotal;
    int finishedTeams;
    ProgressStatus status;
    std::string rescuers;
    int reportedAt;
    int startedAt;
    int finishedAt;
};

// Keeps the history of every emergency and a linked-list event log.
class ProgressTracker {
private:
    const CityMap *map;
    std::string logFile;
    std::vector<ProgressRecord> records;
    LinkedList<std::string> eventLog;
    mutable std::mutex m;

    void writeRecord(std::ostream &out, const ProgressRecord &r) const;

public:
    ProgressTracker(const CityMap *map, const std::string &logFile);

    void reset();
    void add(const Emergency &e);

    // Status only moves forward. Returns true when it really changed.
    bool setStatus(int id, ProgressStatus status, int simMinute);

    void teamSent(int id, const std::string &teamName);

    // One team finished. Returns true when it was the last team needed,
    // and the emergency becomes COMPLETED.
    bool teamFinished(int id, int simMinute);

    void log(int simMinute, const std::string &text);
    int activeCount() const;

    void show() const;
    bool saveReport(const std::string &fileName) const;

    static std::string statusName(ProgressStatus status);
    static std::string clockText(int minutes);
};

#endif

// emergency.cpp
#include "emergency.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>

using namespace std;

static int clampInt(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// ---------------- Emergency ----------------

int Emergency::counter = 0;

Emergency::Emergency()
    : id(0), location(0), type(MEDICAL), intensity(1),
      children(0), adults(0), elders(0), points(0), severity(1),
      reportedAt(0), waitLogged(false) {
    for (int i = 0; i < 3; i++)
        need[i] = remaining[i] = 0;
}

Emergency::Emergency(int loc, EmergencyType t, int inten,
                     int c, int a, int e, int at)
    : id(++counter), location(loc), type(t), intensity(inten),
      children(c), adults(a), elders(e), points(0), severity(1),
      reportedAt(at), waitLogged(false) {

    double weighted = c * CHILD_WEIGHT + a * ADULT_WEIGHT + e * OLD_WEIGHT;

    points = inten * INTENSITY_FACTOR + weighted * PEOPLE_FACTOR;
    severity = levelFromPoints(points);

    planTeams(weighted);
}

int Emergency::levelFromPoints(double p) {
    if (p < 20) return 1;
    if (p < 35) return 2;
    if (p < 50) return 3;
    if (p < 65) return 4;
    return 5;
}

// Decides how many teams of each type this emergency needs.
// The number depends on the incident type, the severity level and the
// weighted number of people (children and old people count more).
void Emergency::planTeams(double weighted) {
    int base = (severity + 1) / 2;   // levels 1..5 -> 1,1,2,2,3
    int amb = 0, fire = 0, police = 0;

    if (type == MEDICAL) {
        amb = clampInt(static_cast<int>(ceil(weighted / 10.0)), 1, 3);
        fire = (severity == 5) ? 1 : 0;
        police = (severity >= 4) ? 1 : 0;
    } else if (type == FIRE) {
        fire = base;
        amb = (severity >= 2)
            ? clampInt(static_cast<int>(ceil(weighted / 15.0)), 1, 3) : 0;
        police = (severity >= 3) ? 1 : 0;
    } else {
        police = base;
        amb = (severity >= 3)
            ? clampInt(static_cast<int>(ceil(weighted / 20.0)), 1, 2) : 0;
    }

    need[0] = amb;
    need[1] = fire;
    need[2] = police;

    for (int i = 0; i < 3; i++)
        remaining[i] = need[i];
}

int Emergency::getNeed(EmergencyType teamType) const {
    return need[teamType - 1];
}

int Emergency::getRemaining(EmergencyType teamType) const {
    return remaining[teamType - 1];
}

int Emergency::needTotal() const {
    return need[0] + need[1] + need[2];
}

int Emergency::remainingTotal() const {
    return remaining[0] + remaining[1] + remaining[2];
}

void Emergency::consume(EmergencyType teamType) {
    if (remaining[teamType - 1] > 0)
        remaining[teamType - 1]--;

    waitLogged = false;   // the situation changed, so log again if it waits
}

bool Emergency::operator<(const Emergency &other) const {
    if (severity != other.severity)
        return severity < other.severity;

    if (points != other.points)
        return points < other.points;

    return id > other.id;
}

ostream &operator<<(ostream &out, const Emergency &e) {
    out << "Emergency #" << e.id
        << " [" << Emergency::typeName(e.type)
        << " | Severity " << e.severity << "]";
    return out;
}

static string countText(const int values[3]) {
    string text;

    for (int i = 0; i < 3; i++) {
        if (values[i] == 0)
            continue;

        if (!text.empty())
            text += ", ";

        text += to_string(values[i]) + " "
              + Emergency::teamLabel(static_cast<EmergencyType>(i + 1));
    }

    return text.empty() ? "none" : text;
}

string Emergency::needsText() const {
    return countText(need);
}

string Emergency::remainingText() const {
    return countText(remaining);
}

string Emergency::summary() const {
    char buffer[200];

    snprintf(buffer, sizeof(buffer),
        "%d people (Child %d, Adult %d, Old %d) | intensity %d | "
        "points %.1f -> severity %d",
        getPeople(), children, adults, elders, intensity, points, severity);

    return string(buffer) + " | teams needed: " + needsText();
}

string Emergency::typeName(EmergencyType type) {
    if (type == MEDICAL) return "Medical";
    if (type == FIRE) return "Fire";
    if (type == CRIME) return "Crime";
    return "Unknown";
}

string Emergency::teamLabel(EmergencyType teamType) {
    if (teamType == MEDICAL) return "ambulance";
    if (teamType == FIRE) return "fire truck";
    return "police car";
}

string Emergency::formulaText() {
    return
        "\n========== SEVERITY FORMULA ==========\n"
        "Age groups : Child, Adult (middle), Old\n"
        "Weights    : Child 1.5 | Adult 1.0 | Old 1.3\n"
        "             (children and old people are more vulnerable)\n\n"
        "weighted people = 1.5 x Child + 1.0 x Adult + 1.3 x Old\n"
        "severity points = 10 x Intensity + 0.5 x weighted people\n\n"
        "Severity level : under 20 -> 1 | under 35 -> 2 | under 50 -> 3\n"
        "                 under 65 -> 4 | 65 or more -> 5\n\n"
        "Teams needed (base = 1,1,2,2,3 for levels 1 to 5):\n"
        "  Medical : ambulances = ceil(weighted / 10), 1 to 3\n"
        "            + 1 fire truck at level 5, 1 police car at level 4+\n"
        "  Fire    : fire trucks = base\n"
        "            ambulances = ceil(weighted / 15), 1 to 3, level 2+\n"
        "            + 1 police car at level 3+\n"
        "  Crime   : police cars = base\n"
        "            ambulances = ceil(weighted / 20), 1 to 2, level 3+\n"
        "======================================\n";
}

void Emergency::resetCounter() {
    counter = 0;
}

// ---------------- EmergencyQueue ----------------

void EmergencyQueue::push(const Emergency &e) {
    lock_guard<mutex> lock(m);
    heap.push(e);
}

vector<Emergency> EmergencyQueue::drainAll() {
    lock_guard<mutex> lock(m);

    vector<Emergency> all;

    while (!heap.empty()) {
        all.push_back(heap.top());
        heap.pop();
    }

    return all;
}

int EmergencyQueue::size() const {
    lock_guard<mutex> lock(m);
    return static_cast<int>(heap.size());
}

void EmergencyQueue::clear() {
    lock_guard<mutex> lock(m);

    while (!heap.empty())
        heap.pop();
}

// ---------------- ProgressTracker ----------------

ProgressTracker::ProgressTracker(const CityMap *cityMap,
                                 const string &file)
    : map(cityMap), logFile(file) {}

string ProgressTracker::statusName(ProgressStatus status) {
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

string ProgressTracker::clockText(int minutes) {
    int t = minutes % (24 * 60);
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%02d:%02d", t / 60, t % 60);
    return string(buffer);
}

void ProgressTracker::reset() {
    lock_guard<mutex> lock(m);

    records.clear();
    eventLog.clear();

    ofstream out(logFile.c_str(), ios::trunc);
    out << "===== LIVE UPDATES (simulated time) =====\n";
}

void ProgressTracker::add(const Emergency &e) {
    lock_guard<mutex> lock(m);

    ProgressRecord r;
    r.id = e.getId();
    r.location = e.getLocation();
    r.type = e.getType();
    r.intensity = e.getIntensity();
    r.severity = e.getSeverity();
    r.points = e.getPoints();
    r.children = e.getChildren();
    r.adults = e.getAdults();
    r.elders = e.getElders();
    r.need[0] = e.getNeed(MEDICAL);
    r.need[1] = e.getNeed(FIRE);
    r.need[2] = e.getNeed(CRIME);
    r.needTotal = e.needTotal();
    r.finishedTeams = 0;
    r.status = REPORTED;
    r.rescuers = "";
    r.reportedAt = e.getReportedAt();
    r.startedAt = -1;
    r.finishedAt = -1;

    records.push_back(r);
}

bool ProgressTracker::setStatus(int id, ProgressStatus status,
                                int simMinute) {
    lock_guard<mutex> lock(m);

    for (ProgressRecord &r : records) {
        if (r.id != id)
            continue;

        if (status <= r.status)
            return false;   // never move backwards

        r.status = status;

        if (status == DISPATCHED && r.startedAt < 0)
            r.startedAt = simMinute;

        return true;
    }

    return false;
}

void ProgressTracker::teamSent(int id, const string &teamName) {
    lock_guard<mutex> lock(m);

    for (ProgressRecord &r : records) {
        if (r.id != id)
            continue;

        if (!r.rescuers.empty())
            r.rescuers += ", ";

        r.rescuers += teamName;
        return;
    }
}

bool ProgressTracker::teamFinished(int id, int simMinute) {
    lock_guard<mutex> lock(m);

    for (ProgressRecord &r : records) {
        if (r.id != id)
            continue;

        r.finishedTeams++;

        if (r.finishedTeams >= r.needTotal) {
            r.status = COMPLETED;
            r.finishedAt = simMinute;
            return true;
        }

        return false;
    }

    return false;
}

void ProgressTracker::log(int simMinute, const string &text) {
    string line = "[" + clockText(simMinute) + "] " + text;

    lock_guard<mutex> lock(m);

    eventLog.push_back(line);

    // Append to the live file; the second window shows this file.
    ofstream out(logFile.c_str(), ios::app);
    out << line << "\n";
}

int ProgressTracker::activeCount() const {
    lock_guard<mutex> lock(m);

    int count = 0;

    for (const ProgressRecord &r : records)
        if (r.status == DISPATCHED || r.status == EN_ROUTE ||
            r.status == ARRIVED)
            count++;

    return count;
}

void ProgressTracker::writeRecord(ostream &out,
                                  const ProgressRecord &r) const {
    out << "\nEmergency #" << r.id
        << " | " << Emergency::typeName(r.type)
        << " | Severity: " << r.severity << "\n";

    out << "Location : " << map->name(r.location) << "\n";

    out << "People   : " << (r.children + r.adults + r.elders)
        << " (Child " << r.children << ", Adult " << r.adults
        << ", Old " << r.elders << ")\n";

    char points[32];
    snprintf(points, sizeof(points), "%.1f", r.points);

    out << "Formula  : intensity " << r.intensity
        << " | points " << points << "\n";

    out << "Teams    : needs ";

    bool any = false;

    for (int i = 0; i < 3; i++) {
        if (r.need[i] == 0)
            continue;

        out << (any ? ", " : "") << r.need[i] << " "
            << Emergency::teamLabel(static_cast<EmergencyType>(i + 1));
        any = true;
    }

    if (!any)
        out << "none";

    out << "\n";

    out << "Status   : " << statusName(r.status) << " ("
        << r.finishedTeams << "/" << r.needTotal << " teams finished)\n";

    out << "Rescuers : "
        << (r.rescuers.empty() ? "Not assigned" : r.rescuers) << "\n";
    out << "Reported : " << clockText(r.reportedAt) << "\n";

    if (r.startedAt >= 0)
        out << "Started  : " << clockText(r.startedAt) << "\n";

    if (r.finishedAt >= 0) {
        out << "Finished : " << clockText(r.finishedAt) << "\n";
        out << "Total    : " << (r.finishedAt - r.reportedAt)
            << " simulated minutes\n";
    } else {
        out << "Total    : in progress\n";
    }
}

void ProgressTracker::show() const {
    lock_guard<mutex> lock(m);

    cout << "\n========== CURRENT EMERGENCY REPORT ==========\n";

    if (records.empty()) {
        cout << "No emergencies have been reported.\n";
        return;
    }

    for (const ProgressRecord &r : records)
        writeRecord(cout, r);

    cout << "\n==============================================\n";
}

bool ProgressTracker::saveReport(const string &fileName) const {
    lock_guard<mutex> lock(m);

    ofstream out(fileName.c_str());

    if (!out)
        return false;

    out << "EMERGENCY RESCUE SIMULATOR - FINAL REPORT\n";

    for (const ProgressRecord &r : records)
        writeRecord(out, r);

    out << "\n--- Event log (" << eventLog.size() << " events) ---\n";

    eventLog.forEach([&out](const string &line) {
        out << line << "\n";
    });

    return true;
}

// ers.h - shared header for the Emergency Rescue Simulator.
// Every .cpp file includes this one file: it holds the constants, the
// structs and the list of functions that the files share.
#ifndef ERS_H
#define ERS_H

#include <string>
#include <mutex>

// ---------- constants ----------
const int NUM_LOCATIONS = 6;
const int NUM_TEAMS = 6;

const long long SIM_START = 8 * 3600;   // simulated clock starts at 08:00
const int TIME_SPEED = 60;              // 1 real second = 60 simulated seconds (1 minute)
const int MIN_PER_KM = 2;               // a team needs 2 simulated minutes per km

const char *const LIVE_FILE = "ers_live.txt";   // feed read by the live window

// ---------- types ----------
enum EmergencyType { MEDICAL = 1, FIRE = 2, CRIME = 3 };

// REPORTED -> (WAITING) -> EN_ROUTE -> ON_SCENE -> COMPLETED
enum Status { REPORTED, WAITING, EN_ROUTE, ON_SCENE, COMPLETED };

struct Emergency {
    int id;
    int location;
    EmergencyType type;
    int severity;                // 1 (low) to 5 (critical)
    Status status;
    int team;                    // index of the assigned team, -1 = none yet
    long long reportedAt;        // all times are simulated seconds
    long long startedAt;         // team dispatched
    long long arrivedAt;         // team reached the location
    long long finishedAt;        // work finished
};

struct RescueTeam {
    std::string name;
    EmergencyType type;
    int location;                // where the team is right now
    bool available;
    int job;                     // id of the emergency it is handling, 0 = none
    int nextStop;                // place it is travelling to
    long long nextAt;            // simulated time of its next event
};

// ---------- citymap.cpp (Part 1 - Paras) ----------
void loadCityMap();
void displayMap(const std::string *marks = nullptr);
std::string getLocationName(int id);
int getDistance(int from, int to);
int getNextStop(int from, int to);
std::string routeText(int from, int to);
void showShortestPath(int from, int to);

// ---------- emergency.cpp (Part 2 - Sahil) ----------
std::string typeName(EmergencyType type);
std::string statusName(Status status);
int addEmergency(int location, EmergencyType type, int severity);
bool serveNextEmergency(int &id);
void putBackEmergency(int id);
Emergency &getEmergency(int id);
int countEmergencies(bool completed);
std::string openEmergencyMarks(int location);
void showReport();

// ---------- dispatch.cpp (Part 3 - Sameer) ----------
extern std::recursive_mutex ersLock;    // protects all shared data

long long simNow();
std::string clockText(long long t);
void liveLog(long long when, const std::string &message);

bool startSimulation();
void stopSimulation();
bool simulationRunning();
void showTeams();
std::string teamName(int index);
void showLiveMap();

#endif

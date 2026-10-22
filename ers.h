
#ifndef ERS_H
#define ERS_H

#include <string>
#include <mutex>


const int NUM_LOCATIONS = 6;
const int NUM_TEAMS = 6;

const long long SIM_START = 8 * 3600;   
const int TIME_SPEED = 60;              
const int MIN_PER_KM = 2;              

const char *const LIVE_FILE = "ers_live.txt";   


enum EmergencyType { MEDICAL = 1, FIRE = 2, CRIME = 3 };


enum Status { REPORTED, WAITING, EN_ROUTE, ON_SCENE, COMPLETED };

struct Emergency {
    int id;
    int location;
    EmergencyType type;
    int severity;              
    Status status;
    int team;                  
    long long reportedAt;        
    long long startedAt;        
    long long arrivedAt;        
    long long finishedAt;      
};

struct RescueTeam {
    std::string name;
    EmergencyType type;
    int location;                
    bool available;
    int job;                     
    int nextStop;                
    long long nextAt;            
};


void loadCityMap();
void displayMap(const std::string *marks = nullptr);
std::string getLocationName(int id);
int getDistance(int from, int to);
int getNextStop(int from, int to);
std::string routeText(int from, int to);
void showShortestPath(int from, int to);


std::string typeName(EmergencyType type);
std::string statusName(Status status);
int addEmergency(int location, EmergencyType type, int severity);
bool serveNextEmergency(int &id);
void putBackEmergency(int id);
Emergency &getEmergency(int id);
int countEmergencies(bool completed);
std::string openEmergencyMarks(int location);
void showReport();


extern std::recursive_mutex ersLock;   

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

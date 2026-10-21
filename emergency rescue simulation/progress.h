// progress.h
#ifndef PROGRESS_H
#define PROGRESS_H

#include "emergency.h"
#include <string>

enum ProgressStatus {
    REPORTED,
    WAITING,
    DISPATCHED,
    EN_ROUTE,
    ARRIVED,
    COMPLETED
};

struct EmergencyProgress {
    int emergencyId;
    int location;
    EmergencyType type;
    int severity;
    ProgressStatus status;
    int teamIndex;
    long long reportedAt;
    long long startedAt;
    long long finishedAt;
};

void initProgress();
void registerEmergency(const Emergency &emergency);
void markWaiting(const Emergency &emergency);
void markDispatched(const Emergency &emergency, int teamIndex);
void markEnRoute(const Emergency &emergency, int teamIndex);
void markArrived(const Emergency &emergency, int teamIndex);
void markCompleted(const Emergency &emergency, int teamIndex);
void showProgress();

long long currentTimeSeconds();
std::string progressStatusName(ProgressStatus status);

#endif

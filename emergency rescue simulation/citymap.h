// citymap.h
#ifndef CITYMAP_H
#define CITYMAP_H

#include <string>

const int NUM_LOCATIONS = 6;

void loadCityMap();
void displayMap();

std::string getLocationName(int id);

int getDistance(int from, int to);
int getRoadDistance(int from, int to);
int getNextLocation(int from, int to);

void showShortestPath(int from, int to);

#endif

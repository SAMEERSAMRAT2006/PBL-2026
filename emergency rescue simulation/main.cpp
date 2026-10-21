// main.cpp
// Simple menu for the Emergency Rescue Simulator.
#include <iostream>
#include <cstdlib>
#include <limits>

#include "citymap.h"
#include "emergency.h"
#include "dispatch.h"
#include "progress.h"
#include "simulation.h"

using namespace std;

static int readInt(const string &prompt, int low, int high) {
    int value;

    while (true) {
        cout << prompt;

        if (cin >> value && value >= low && value <= high)
            return value;

        if (cin.eof()) {
            cout << "\nInput ended.\n";
            stopSimulation();
            exit(0);
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Please enter a number from "
             << low << " to " << high << ".\n";
    }
}

static void showLocations() {
    for (int i = 0; i < NUM_LOCATIONS; i++)
        cout << i << " = " << getLocationName(i) << "\n";
}

static void reportEmergency() {
    if (!simulationRunning()) {
        cout << "\nStart the simulation first.\n";
        return;
    }

    cout << "\n========== REPORT EMERGENCY ==========\n";

    showLocations();

    int location = readInt(
        "Location (0-5): ", 0, NUM_LOCATIONS - 1);

    int type = readInt(
        "Type (1=Medical, 2=Fire, 3=Crime): ", 1, 3);

    int severity = readInt(
        "Severity (1=Low, 2=Moderate, 3=Serious, "
        "4=High, 5=Critical): ", 1, 5);

    addEmergency(
        location,
        static_cast<EmergencyType>(type),
        severity);
}

static void printMenu() {
    cout << "\n\n============================================\n";
    cout << "       EMERGENCY RESCUE SIMULATOR\n";
    cout << "============================================\n";

    cout << "Simulation: "
         << (simulationRunning() ? "RUNNING" : "STOPPED")
         << "\n";

    cout << "Pending emergencies: "
         << pendingEmergencies() << "\n\n";

    cout << "1. Start simulation\n";
    cout << "2. Report emergency\n";
    cout << "3. View current emergency report\n";
    cout << "4. View city map\n";
    cout << "5. View rescue teams\n";
    cout << "6. Exit\n";
}

int main() {
    int choice;

    do {
        printMenu();

        choice = readInt("Enter choice: ", 1, 6);

        switch (choice) {
            case 1:
                startSimulation();
                break;

            case 2:
                reportEmergency();
                break;

            case 3:
                showProgress();
                break;

            case 4:
                if (simulationRunning())
                    displayMap();
                else
                    cout << "\nStart the simulation first.\n";
                break;

            case 5:
                if (simulationRunning())
                    showTeams();
                else
                    cout << "\nStart the simulation first.\n";
                break;

            case 6:
                stopSimulation();
                cout << "Goodbye!\n";
                break;
        }

    } while (choice != 6);

    return 0;
}

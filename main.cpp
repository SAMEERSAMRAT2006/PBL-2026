
#include <iostream>
#include <cstdlib>
#include <limits>
#include <stdexcept>

#include "citymap.h"
#include "emergency.h"
#include "simulation.h"

using namespace std;

static Simulation sim;

static int readInt(const string &prompt, int low, int high) {
    int value;

    while (true) {
        cout << prompt;

        if (cin >> value && value >= low && value <= high)
            return value;

        if (cin.eof()) {
            cout << "\nInput ended.\n";
            sim.stop();
            exit(0);
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Please enter a number from "
             << low << " to " << high << ".\n";
    }
}

static void showLocations() {
    CityMap names;

    for (int i = 0; i < NUM_LOCATIONS; i++)
        cout << i << " = " << names.describePlace(i) << "\n";
}

static void reportEmergency() {
    if (!sim.isRunning()) {
        cout << "\nStart the simulation first.\n";
        return;
    }

    cout << "\n========== REPORT EMERGENCY ==========\n";
    showLocations();

    CityMap city;

    int location = readInt("Location (0-5): ", 0, NUM_LOCATIONS - 1);
    int type = readInt("Type (1=Medical, 2=Fire, 3=Crime): ", 1, 3);
    int intensity = readInt(
        "Incident intensity (1=Minor, 2=Moderate, 3=Serious, "
        "4=Major, 5=Catastrophic): ", 1, 5);

    int present = city.profile(location).people;

    int people = readInt(
        "People affected (1-" + to_string(present) + "): ", 1, present);


    sim.report(location, static_cast<EmergencyType>(type),
               intensity, people);
}

static void routeFinder() {
    cout << "\n========== ROUTE FINDER ==========\n";
    showLocations();

    int from = readInt("From (0-5): ", 0, NUM_LOCATIONS - 1);
    int to = readInt("To (0-5): ", 0, NUM_LOCATIONS - 1);

    sim.routeFinder(from, to);
}

static void printMenu() {
    cout << "\n\n============================================\n";
    cout << "       EMERGENCY RESCUE SIMULATOR\n";
    cout << "============================================\n";
    cout << "Simulation: " << (sim.isRunning() ? "RUNNING" : "STOPPED")
         << " | Pending: " << sim.pending()
         << " | In progress: " << sim.active() << "\n\n";

    cout << "1. Start simulation\n";
    cout << "2. Report emergency\n";
    cout << "3. View current emergency report\n";
    cout << "4. View city map\n";
    cout << "5. View rescue teams\n";
    cout << "6. Route finder (Dijkstra and BFS)\n";
    cout << "7. Open live update window\n";
    cout << "8. Save report to file\n";
    cout << "9. Show severity formula\n";
    cout << "10. Exit\n";
}

int main() {
    int choice;

    do {
        printMenu();
        choice = readInt("Enter choice: ", 1, 10);

        try {
            switch (choice) {
                case 1: sim.start(); break;
                case 2: reportEmergency(); break;
                case 3: sim.showReport(); break;
                case 4: sim.showMap(); break;
                case 5: sim.showTeams(); break;
                case 6: routeFinder(); break;
                case 7: sim.openLiveWindow(); break;
                case 8: sim.saveReport(); break;
                case 9: sim.showFormula(); break;
                case 10:
                    sim.stop();
                    cout << "Goodbye!\n";
                    break;
            }
        } catch (const exception &ex) {
            cout << "\nError: " << ex.what() << "\n";
        }

    } while (choice != 10);

    return 0;
}

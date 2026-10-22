
#include "ers.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <thread>
#include <chrono>

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
        cout << "Please enter a number from " << low << " to " << high << ".\n";
    }
}

static void showLocations() {
    for (int i = 0; i < NUM_LOCATIONS; i++)
        cout << i << " = " << getLocationName(i) << "\n";
}

static void newEmergency(int location, int type, int severity) {
    int id = addEmergency(location, static_cast<EmergencyType>(type), severity);

    cout << "\nEmergency #" << id << " reported at " << getLocationName(location)
         << " (" << typeName(static_cast<EmergencyType>(type)) << ", severity " << severity << ").\n";
    cout << "Watch the live window for updates.\n";
}

static void reportEmergency() {
    cout << "\n========== REPORT EMERGENCY ==========\n";
    showLocations();

    int location = readInt("Location (0-5): ", 0, NUM_LOCATIONS - 1);
    int type = readInt("Type (1=Medical, 2=Fire, 3=Crime): ", 1, 3);
    int severity = readInt("Severity (1=Low ... 5=Critical): ", 1, 5);

    newEmergency(location, type, severity);
}

static void findRoute() {
    cout << "\n";
    showLocations();

    int from = readInt("From (0-5): ", 0, NUM_LOCATIONS - 1);
    int to = readInt("To (0-5): ", 0, NUM_LOCATIONS - 1);

    showShortestPath(from, to);
}

static void openLiveWindow(const string &exe) {
#ifdef _WIN32
    string command = "start \"ERS Live Window\" \"" + exe + "\" monitor";
#else
    string command = "(x-terminal-emulator -e '" + exe + "' monitor || gnome-terminal -- '" + exe +
                     "' monitor || xterm -e '" + exe + "' monitor) >/dev/null 2>&1 &";
#endif
    system(command.c_str());

    cout << "A live window should have opened.\n";
    cout << "If it did not, open a second terminal in this folder and run:  " << exe << " monitor\n";
}


static void runMonitor() {
    cout << "=========== EMERGENCY RESCUE SIMULATOR - LIVE WINDOW ===========\n";
    cout << "Every emergency update appears here as it happens.\n";
    cout << "Times shown are city time (1 real second = 1 minute).\n\n";

    streamoff position = 0;
    bool stopped = false;

    while (!stopped) {
        ifstream in(LIVE_FILE, ios::binary);

        if (in) {
            in.seekg(0, ios::end);
            if (in.tellg() < position)       
                position = 0;
            in.seekg(position);

            string line;
            while (getline(in, line)) {
                if (in.eof())               
                    break;
                cout << line << "\n";
                position += line.size() + 1;
                if (line.find("SIMULATION STOPPED") != string::npos)
                    stopped = true;
            }
        }

        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    cout << "\nPress Enter to close this window...";
    cin.get();
}

static void printMenu() {
    cout << "\n\n============================================\n";
    cout << "       EMERGENCY RESCUE SIMULATOR\n";
    cout << "============================================\n";

    if (simulationRunning()) {
        cout << "Simulation: RUNNING | City time: " << clockText(simNow()) << "\n";
        cout << "Open emergencies: " << countEmergencies(false)
             << " | Completed: " << countEmergencies(true) << "\n";
    } else {
        cout << "Simulation: STOPPED\n";
    }

    cout << "\n1. Start simulation\n";
    cout << "2. Report emergency\n";
    cout << "3. Random emergency (demo)\n";
    cout << "4. View emergency report\n";
    cout << "5. View city map\n";
    cout << "6. View rescue teams\n";
    cout << "7. Find route between two places\n";
    cout << "8. Exit\n";
}

int main(int argc, char *argv[]) {
    string exe = (argc > 0) ? argv[0] : "rescue";

    if (argc > 1 && string(argv[1]) == "monitor") {
        runMonitor();
        return 0;
    }

    srand(static_cast<unsigned>(time(nullptr)));
    int choice;

    do {
        printMenu();
        choice = readInt("Enter choice: ", 1, 8);

        if (choice >= 2 && choice <= 7 && !simulationRunning()) {
            cout << "\nStart the simulation first (option 1).\n";
            continue;
        }

        switch (choice) {
            case 1:
                if (startSimulation())
                    openLiveWindow(exe);
                break;
            case 2: reportEmergency(); break;
            case 3: newEmergency(rand() % NUM_LOCATIONS, rand() % 3 + 1, rand() % 5 + 1); break;
            case 4: showReport(); break;
            case 5: showLiveMap(); break;
            case 6: showTeams(); break;
            case 7: findRoute(); break;
            case 8:
                stopSimulation();
                cout << "Goodbye!\n";
                break;
        }
    } while (choice != 8);

    return 0;
}

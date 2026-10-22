# Emergency Rescue Simulator (ERS)

**Team "Avengers"** | Team ID: DSCPP-III-2026-T237 | Domain: DS and OOPS with C++ | Semester 3
**Mentor:** Mr. Upendra Singh Aswal

A terminal-based C++ program that simulates how a city's emergency services (ambulance, fire brigade and police)
respond to emergencies. You report emergencies as they happen, and the program decides who is sent first, which team
goes, and which roads it takes. Every step is shown live in a second window.

---

## The problem it tackles

In a real city, emergencies do not come one at a time, and the number of rescue teams is limited. This causes
four problems:

1. **Which emergency first?** A heart attack should be served before a minor accident, even if the accident was reported earlier.
2. **Which team to send?** Sending a team that is far away wastes time. The nearest free team of the right type should go (ambulance for medical, fire truck for fire, police car for crime).
3. **What if all teams are busy?** The emergency must wait in line and be served the moment a team becomes free, without being forgotten.
4. **How long did it take?** Response time (report to arrival) is what decides whether a rescue was good. Nobody can improve what they cannot measure.

ERS lets us run these situations on a small city map and see the result: who waited, how long each rescue took,
and the average response time.

## How ERS solves it

| Problem | Solution in the program | Data structure |
|---------|------------------------|----------------|
| Most severe first | Emergencies wait in a priority queue ordered by severity (1 to 5); equal severity is served oldest first | Priority queue |
| Nearest team | Distance between places is read from a preloaded table; the nearest free team of the matching type is sent | 2D array + array of structs |
| All teams busy | The emergency gets the status WAITING and goes back into the queue; it is sent when a team becomes free | Priority queue |
| Measuring | Every emergency records its reported, dispatch, arrival and finish time; the report shows the average response time | Vector of records |
| Understanding the city | The map is drawn as a picture with teams and open emergencies marked on it | 2D character grid |

**Phase-II note:** this phase uses **no BFS and no Dijkstra**. The shortest distances and the first road to take for
every pair of places are fixed tables, and the program only looks them up. Dijkstra comes in the next phase.

## Main features

- **Start first, then report:** the simulation is started with option 1, then emergencies can be reported at any time.
- **City map as a picture** with road distances, rescue teams (`A1`, `F2`, `P1`, `*` = busy) and open emergencies (`!3`).
- **Separate live window** that prints every update as it happens: `NEW`, `SEND`, `MOVE`, `ARRV`, `DONE`, `WAIT`.
- **City clock:** 1 real second = 1 city minute, so a rescue of 20 to 60 city minutes takes 20 to 60 real seconds. The program does not need to run for half an hour, but it looks like a lot of time passed.
- **Several teams work at the same time.**
- **Progress report:** status, assigned team, reported/dispatch/arrival/finish time, response time and average response time.
- **Random emergency** option for quick demos, and a **route finder** between any two places.

---

## Team and who did what

| Member | Part | File | Work |
|--------|------|------|------|
| **Sahil Sinha** | Part 1 - City Map | `src/citymap.cpp` | The city: 6 places and 8 two-way roads stored in a 2D array. The fixed distance and next-stop tables. The **map picture** (`displayMap`). Route text and the route finder (`routeText`, `showShortestPath`). |
| **Paras Chauhan** | Part 2 - Emergency Queue | `src/emergency.cpp` | The **priority queue** of waiting emergencies (`addEmergency`, `serveNextEmergency`, `putBackEmergency`). The record of every emergency and its status. The **progress report** with response times (`showReport`). |
| **Sameer Samrat** (team lead) | Part 3 - Dispatch, Clock, Menu | `src/dispatch.cpp`, `src/main.cpp` | Rescue teams and **nearest-team dispatch** (`dispatchRescueTeam`). The **city clock** and the background engine that moves teams stop by stop. The live feed. The **menu** and the **live window**. |
| Everyone | Shared | `src/ers.h` | Constants, the `Emergency` and `RescueTeam` structs and the list of functions the files share. |

How the parts fit together: Paras's queue gives the most urgent emergency, Sameer's dispatch picks the nearest free
team using Sahil's distance table, and Sameer's engine moves that team along Sahil's routes while Paras's records
store the times.

## The functions

| # | Function | File | Data structure |
|---|----------|------|----------------|
| 1 | `loadCityMap()` | citymap.cpp | 2D array |
| 2 | `displayMap()` | citymap.cpp | 2D character grid |
| 3 | `addEmergency()` | emergency.cpp | Priority queue |
| 4 | `serveNextEmergency()` | emergency.cpp | Priority queue |
| 5 | `showShortestPath()` | citymap.cpp | Fixed distance and next-stop tables |
| 6 | `dispatchRescueTeam()` | dispatch.cpp | Array of structs |

Extra functions: `showReport()` (emergency.cpp), `showTeams()`, `showLiveMap()`, the engine and the clock (dispatch.cpp),
`runMonitor()` and `openLiveWindow()` (main.cpp).

## Files

```
revisedproj/
  src/
    ers.h            shared header
    citymap.cpp      Part 1 - Sahil
    emergency.cpp    Part 2 - Paras
    dispatch.cpp     Part 3 - Sameer
    main.cpp         Part 3 - Sameer
  README.md
  ERS_Code_Explanation.pdf    line-by-line explanation of every file
  .gitignore
```

## Build and run

Open a terminal **in the `revisedproj` folder** (the folder that contains `src`), then:

```text
g++ -std=c++11 -Wall -pthread -o rescue.exe src/main.cpp src/citymap.cpp src/emergency.cpp src/dispatch.cpp
.\rescue.exe
```

On Linux or macOS: `-o rescue` and `./rescue`.
Needs a compiler with thread support. On Windows use MinGW-w64 with POSIX threads (MSYS2 or WinLibs).

## Menu

| # | Option | What it does |
|---|--------|--------------|
| 1 | Start simulation | starts the city clock and opens the live window (do this first) |
| 2 | Report emergency | choose place, type and severity |
| 3 | Random emergency (demo) | random place, type and severity |
| 4 | View emergency report | status, team, times, average response time |
| 5 | View city map | the map picture with teams and open emergencies |
| 6 | View rescue teams | where each team is and whether it is busy |
| 7 | Find route | route, km and travel time between two places |
| 8 | Exit | stops the simulation |

## The live window

Option 1 opens a second window that runs `rescue monitor`. If it does not open, open another terminal in the same
folder and run `.\rescue.exe monitor`. The control window writes every event to `ers_live.txt` and the live window
prints the new lines about every 0.3 seconds.

Time rules (all in `ers.h` / `dispatch.cpp`): clock speed `TIME_SPEED` = 60, driving 2 minutes per km,
work at the scene 10 + 4 x severity minutes.

## Demo flow for evaluation

1. Option 1 - start; the live window opens
2. Option 5 - show the map picture
3. Option 2 - report 3 or 4 emergencies (two of the same type with different severity)
4. Watch the live window: SEND, MOVE, ARRV, DONE lines (and WAIT if all teams of a type are busy)
5. Option 4 - report, option 6 - teams, option 5 - map again with markers
6. Option 7 - route, for example 0 to 5

---

## GitHub guide

### 0. One-time setup (every member)

```
git --version
git config --global user.name  "Your Name"
git config --global user.email "your-github-email@example.com"
```

### 1. Team lead (Sameer) creates the repo

1. On github.com click **New repository**. Name: `emergency-rescue-simulator`. Choose Private. Do **not** tick "Add a README".
2. In the project folder run:

```
git init
git add README.md .gitignore src/ers.h
git commit -m "Add README, .gitignore and shared header"
git branch -M main
git remote add origin https://github.com/<your-username>/emergency-rescue-simulator.git
git push -u origin main
```

3. On GitHub: **Settings > Collaborators > Add people**. Add Paras and Sahil by username.

### 2. Each member pushes their own part on their own branch

Branch names: `part1-citymap` (Sahil), `part2-emergency-queue` (Paras), `part3-dispatch-main` (Sameer).
Example for Sahil:

```
git clone https://github.com/<your-username>/emergency-rescue-simulator.git
cd emergency-rescue-simulator
git checkout -b part1-citymap
mkdir -p src
# copy your file into src/
git add src/citymap.cpp
git commit -m "Add city map module (Part 1)"
git push -u origin part1-citymap
```

Paras adds `src/emergency.cpp`; Sameer adds `src/dispatch.cpp` and `src/main.cpp`.

### 3. Pull request and merge

1. On GitHub open the repo and click **Compare & pull request** on your branch.
2. The lead reviews and clicks **Merge pull request**.
3. After all 3 PRs are merged, everyone runs:

```
git checkout main
git pull
```

Then build with the g++ command above to check the full program.

### 4. Daily workflow

```
git pull                      # get teammates' latest work first
# ... edit your own file ...
git status                    # see what changed
git add src/yourfile.cpp
git commit -m "Short message about the change"
git push
```

### 5. Common problems

| Problem | Fix |
|---------|-----|
| Password rejected on push | GitHub no longer accepts passwords. Use a Personal Access Token (Settings > Developer settings > Tokens), or sign in through VS Code / GitHub Desktop. |
| `rejected ... non-fast-forward` | `git pull --rebase` then `git push` |
| Merge conflict | Edit only your own files. If one happens, open the file, keep the right lines, remove the `<<<<`, `====`, `>>>>` markers, then `git add` and `git commit`. |
| Pushed `rescue.exe` or `ers_live.txt` by mistake | The `.gitignore` blocks both. If already committed: `git rm --cached rescue.exe` and commit. |

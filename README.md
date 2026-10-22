# Emergency Rescue Simulator (ERS)

Team "Avengers" | Team ID DSCPP-III-2026-T237 | DS and OOPS with C++ | Mentor: Mr. Upendra Singh Aswal

Terminal-based C++ prototype (Phase-II). The city map is preloaded, and the shortest distances and routes are
**fixed tables**. A city clock runs 60x faster than real time, and every
update shows up in a **separate live window**.

Full line-by-line explanation: `ERS_Code_Explanation.pdf`

## Files (5 source files)

| File | Owner | What it gives |
|------|-------|---------------|
| `src/ers.h` | everyone | constants, `Emergency` and `RescueTeam` structs, function list |
| `src/citymap.cpp` | Paras Chauhan (Part 1) | map data, **map as a picture**, distance and route tables |
| `src/emergency.cpp` | Sahil Sinha (Part 2) | priority queue, emergency history, progress report |
| `src/dispatch.cpp` | Sameer Samrat (Part 3) | teams, nearest-team dispatch, city clock, background engine, live feed |
| `src/main.cpp` | Sameer Samrat (Part 3) | menu and the live window (`rescue monitor`) |

## Menu

| # | Option | What it does |
|---|--------|--------------|
| 1 | Start simulation | starts the city clock and opens the live window (must be done first) |
| 2 | Report emergency | choose place, type, severity; goes into the priority queue |
| 3 | Random emergency (demo) | random place, type and severity |
| 4 | View emergency report | status, team, report/arrival/finish times, average response time |
| 5 | View city map | the map as a picture with teams (A, F, P) and open emergencies (!n) |
| 6 | View rescue teams | where each team is and whether it is busy |
| 7 | Find route | route, km and travel time between two places (table lookup) |
| 8 | Exit | stops the simulation |

## Build and run

```text
g++ -std=c++11 -Wall -pthread -o rescue src/main.cpp src/citymap.cpp src/emergency.cpp src/dispatch.cpp
./rescue          # on Windows: rescue.exe
```

Needs a compiler with thread support. On Windows use MinGW-w64 with POSIX threads (MSYS2 or WinLibs).

## The live window

Pressing option 1 opens a second window that runs `rescue monitor`. If it does not open by itself, open another
terminal in the same folder and run `rescue monitor` (Windows: `rescue.exe monitor`). The control window writes
every event to `ers_live.txt`; the live window prints new lines about every 0.3 seconds.

City time: 1 real second = 1 city minute (change `TIME_SPEED` in `ers.h`). Driving takes 2 minutes per km, work at the
scene takes 10 + 4 x severity minutes, so one rescue takes roughly 20 to 60 real seconds.

## Demo flow for evaluation

1. Option 1 - start; the live window opens
2. Option 5 - show the map picture
3. Option 2 - report 3 or 4 emergencies (two of the same type, different severity)
4. Watch the live window: SEND, MOVE, ARRV, DONE lines (and WAIT if all teams of a type are busy)
5. Option 4 - report; option 6 - teams; option 5 - map again with markers
6. Option 7 - route, for example 0 to 5

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

Paras (use `part2-emergency-queue` for Sahil and `part3-dispatch-main` for Sameer; the shared header `src/ers.h` is pushed by the lead in step 1):

```
git clone https://github.com/<your-username>/emergency-rescue-simulator.git
cd emergency-rescue-simulator
git checkout -b part1-citymap
mkdir -p src
# copy your files into src/
git add src/citymap.cpp
git commit -m "Add city map module (Part 1)"
git push -u origin part1-citymap
```

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
# ... edit your own files ...
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
| Pushed the compiled `rescue.exe` or `ers_live.txt` by mistake | The `.gitignore` blocks both. If already committed: `git rm --cached rescue.exe` and commit. |

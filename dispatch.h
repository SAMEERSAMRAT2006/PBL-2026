// dispatch.h
// Sameer's part (1/3): rescue teams and the dispatcher.
//
// OOPS: Abstraction (abstract base class), inheritance, runtime
//       polymorphism (virtual functions), virtual destructor,
//       constructors/destructors, dynamic memory, protected members.
// DSA : Dijkstra (through CityMap) to find the nearest free team.
#ifndef DISPATCH_H
#define DISPATCH_H

#include <string>
#include <vector>
#include <mutex>

#include "citymap.h"
#include "emergency.h"

// Abstract class: it has pure virtual functions, so no object of
// RescueTeam itself can be created.
class RescueTeam {
protected:
    std::string name;
    EmergencyType type;
    int baseLocation;
    int location;
    bool available;

public:
    RescueTeam(const std::string &name, EmergencyType type, int baseLocation);
    virtual ~RescueTeam();

    virtual int serviceMinutes() const = 0;      // time spent at the scene
    virtual std::string vehicle() const = 0;
    virtual std::string action() const = 0;

    const std::string &getName() const { return name; }
    EmergencyType getType() const { return type; }
    int getBase() const { return baseLocation; }
    int getLocation() const { return location; }
    bool isAvailable() const { return available; }

    void setLocation(int loc) { location = loc; }
    void setAvailable(bool value) { available = value; }
    void resetToBase();
};

class Ambulance : public RescueTeam {
public:
    Ambulance(const std::string &name, int baseLocation);
    int serviceMinutes() const override;
    std::string vehicle() const override;
    std::string action() const override;
};

class FireTruck : public RescueTeam {
public:
    FireTruck(const std::string &name, int baseLocation);
    int serviceMinutes() const override;
    std::string vehicle() const override;
    std::string action() const override;
};

class PoliceCar : public RescueTeam {
public:
    PoliceCar(const std::string &name, int baseLocation);
    int serviceMinutes() const override;
    std::string vehicle() const override;
    std::string action() const override;
};

class Dispatcher {
private:
    const CityMap &map;
    std::vector<RescueTeam *> teams;   // base-class pointers: polymorphism
    mutable std::mutex m;

public:
    explicit Dispatcher(const CityMap &map);
    ~Dispatcher();

    Dispatcher(const Dispatcher &) = delete;
    Dispatcher &operator=(const Dispatcher &) = delete;

    // Picks the nearest free team of the given type (by Dijkstra km),
    // marks it busy and returns its index. Returns -1 if none is free.
    // Called once for every team an emergency needs.
    int assign(EmergencyType teamType, int location, Route &route);

    void release(int index, int newLocation);
    void reset();
    void show() const;

    std::string teamName(int index) const;
    int serviceMinutes(int index) const;
    std::string action(int index) const;
};

#endif

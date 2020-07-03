#pragma once

#include <vector>
#include <string>

struct ExtraSchedule {
    std::vector<int> hours;
    char shift;
    std::vector<int> days;
};

class ClassSchedule
{
private:
    std::vector<int> hours;
    char shift;
    std::vector<int> days;
    
    ExtraSchedule spillover;
    bool hasSpillover;

public:
    ClassSchedule();
    ClassSchedule(std::string str);
    bool hasConflict(ClassSchedule other);

    std::vector<int> getHours() { return hours; }
    std::vector<int> getDays() { return days; }
    char getShift() { return shift; }
    
    ExtraSchedule getSpillover() { return spillover; }
    bool getHasSpillover() { return hasSpillover; }
};
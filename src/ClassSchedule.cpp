#include "../include/ClassSchedule.h"

ClassSchedule::ClassSchedule()
{
    shift = '0';
    hours = std::vector<int>(0);
    days = std::vector<int>(0);
}

ClassSchedule::ClassSchedule(std::string str)
{
    int dashPos = str.size();

    // Check for dash, to create spillover hours
    hasSpillover = false;
    for(int j = 0; j < str.size(); j++)
    {
        if(str[j] == '-')
        {   
            hasSpillover = true;
            dashPos = j;

            int shiftPos;
            for(int i = j; i < str.size(); i++)
            {
                if(str[i] == 'M' || str[i] == 'T' || str[i] == 'N')
                    shiftPos = i;
            }

            for(int i = 0; i < shiftPos; i++)
                spillover.days.push_back(str[i]);

            spillover.shift = str[shiftPos];

            for(int i = shiftPos+1; i < str.size(); i++)
                spillover.hours.push_back(str[i]);
        }
    }

    // Check to see if class doesn't have hours
    if(str[0] == 'E')
    {
        days.push_back(0);
        hours.push_back(0);
        shift = 'E';
    }
    // Fill actual hours
    else
    {
        int shiftPos = 0;
        for(int i = 0; i < dashPos; i++)
        {
            if(str[i] == 'M' || str[i] == 'T' || str[i] == 'N')
                shiftPos = i;
        }

        for(int i = 0; i < shiftPos; i++)
            days.push_back(str[i]);

        shift = str[shiftPos];

        for(int i = shiftPos+1; i < dashPos; i++)
            hours.push_back(str[i]);
    }
}

bool ClassSchedule::hasConflict(ClassSchedule other)
{
    if (shift == 'E') return false;

    bool sameDay[4] = {false};
    bool sameShift[4] = {false};
    bool sameHours[4] = {false};

    std::vector<int> otherDays = other.getDays(); 
    std::vector<int> otherHours = other.getHours();
    ExtraSchedule otherSpillover = other.getSpillover();

    for(int a : days) {
        for(int b : otherDays) {
            if(a == b) {
                sameDay[0] = true;
                break;
            }
        }
        if(sameDay[0]) break;
    }

    for(int a : hours) {
        for(int b : otherHours) {
            if(a == b) {
                sameHours[0] = true;
                break;
            }
        }
        if(sameHours) break;
    }

    if(shift == other.getShift()) sameShift[0] = true;

    if(hasSpillover)
    {
        for(int a : spillover.days) {
            for(int b : otherDays) {
                if(a == b) {
                    sameDay[1] = true;
                    break;
                }
            }
            if(sameDay[1]) break;
        }

        for(int a : spillover.hours) {
            for(int b : otherHours) {
                if(a == b) {
                    sameHours[1] = true;
                    break;
                }
            }
            if(sameHours[1]) break;
        }

        if(spillover.shift == other.getShift()) sameShift[1] = true;
    }

    if(other.getHasSpillover())
    {
        for(int a : days) {
            for(int b : otherSpillover.days) {
                if(a == b) {
                    sameDay[2] = true;
                    break;
                }
            }
            if(sameDay[2]) break;
        } 

        for(int a : hours) {
            for(int b : otherSpillover.hours) {
                if(a == b) {
                    sameHours[2] = true;
                    break;
                }
            }
            if(sameHours[2]) break;
        }

        if(shift == otherSpillover.shift) sameShift[2] = true;
    }

    if(hasSpillover && other.getHasSpillover())
    {
        for(int a : spillover.days) {
            for(int b : otherSpillover.days) {
                if(a == b) {
                    sameDay[3] = true;
                    break;
                }
            }
            if(sameDay[3]) break;
        } 

        for(int a : spillover.hours) {
            for(int b : otherSpillover.hours) {
                if(a == b) {
                    sameHours[3] = true;
                    break;
                }
            }
            if(sameHours[3]) break;
        }

        if(spillover.shift == otherSpillover.shift) sameShift[3] = true;
    }
    
    for(int i = 0; i < 4; i++)
    {
        if(sameDay[i] && sameHours[i] && sameShift[i])
        {
            return true;
        }
    }
    return false;
}
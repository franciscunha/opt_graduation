#pragma once

#include <vector>
#include <string>
#include "../include/DataException.h"

class Data
{
private:
    int nClasses;
    int maxHours;
    int minSemesters;
    int minOptHours;
    std::vector<int> classHours;
    std::vector<std::vector<int>> prerequisite;
    std::vector<std::vector<int>> corequisite; 

    std::vector<int> optionalClasses;
    std::vector<int> mandatoryClasses;
    std::vector<int> finalClasses;
    std::vector<int> preFinalClasses;

public:
    int getNClasses() { return nClasses; }
    
    int getMaxHours() { return maxHours; }
    int getMinSemesters() { return minSemesters; }
    int getMinOptHours() { return minOptHours; }
    
    int getClassHours(int i) { return classHours[i]; }
    int isPreReq(int i, int j) { return prerequisite[i][j]; }
    int isCoReq(int i, int j) { return corequisite[i][j]; }

    std::vector<int> getClasses() {
        std::vector<int> classes;
        for(int i = 0; i < nClasses; i++)
            classes.push_back(i);
        return classes;
    }
    std::vector<int> getSemesters() { return getClasses(); }
    std::vector<int> getOptClasses() { return optionalClasses; }
    std::vector<int> getManClasses() { return mandatoryClasses; }
    std::vector<int> getFinClasses() { return finalClasses; }
    std::vector<int> getPreFinClasses() { return preFinalClasses; }

    Data(std::string filePath);
};
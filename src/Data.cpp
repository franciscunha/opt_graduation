#include "../include/Data.h"
#include "../include/DataException.h"
#include "../include/ClassSchedule.h"
#include <fstream>
#include <iostream>

struct Pair {
	int i;
	int j;
};

Data::Data(std::string filePath)
{
	std::ifstream file(filePath);
	if(!file.is_open()) throw DataException();

	std::cout << "File opened" << std::endl;

	file >> nClasses;
	file >> maxHours;
	file >> minHours;
	file >> minOptHours;
	file >> maxSemesters;

	int minNSems, takenSems;
	file >> minNSems;
	file >> takenSems;
	minSemesters = minNSems - takenSems;
	
	if(file.bad() || file.fail()) throw DataException();

	std::cout << "There are " << nClasses << " classes" << std::endl;
	std::cout << std::endl;
	
    classHours = std::vector<int>(nClasses, 0);

	std::vector<ClassSchedule> classSchedules = std::vector<ClassSchedule>(nClasses, ClassSchedule());
	std::vector<int> classCategories = std::vector<int>(nClasses, 0);
	std::vector<Pair> coreqPairs;
	std::vector<Pair> prereqPairs;

	for(int i = 0; i < nClasses; i++)
	{
		std::cout << "Reading class " << i << " details" << std::endl;

		int classID = i;
		file >> classID;
		file >> classHours[classID];

		std::string scheduleStr;
		file >> scheduleStr;
		classSchedules[classID] = ClassSchedule(scheduleStr);

		file >> classCategories[classID];

		// Fill vector of corequisite pairs
		int nCoReq = 0;
		file >> nCoReq;

		std::cout << "Has " << nCoReq << " corequisites" << std::endl;

		for(int j = 0; j < nCoReq; j++)
		{
			int coReq;
			file >> coReq;
			if(file.bad() || file.fail()) throw DataException();

			std::cout << "Read " << j+1 << " corequisites" << std::endl;
			
			Pair coreqPair;
			coreqPair.i = classID;
			coreqPair.j = coReq;

			coreqPairs.push_back(coreqPair);
		}

		// Fill vector of prerequisite pairs
		int nPreReq = 0;
		file >> nPreReq;

		std::cout << "Has " << nPreReq << " prerequisites" << std::endl;

		for(int j = 0; j < nPreReq; j++)
		{
			int preReq;
			file >> preReq;
			if(file.bad() || file.fail()) throw DataException();

			std::cout << "Read " << j+1 << " prerequisites" << std::endl;
			
			Pair prereqPair;
			prereqPair.j = classID;
			prereqPair.i = preReq;

			prereqPairs.push_back(prereqPair);
		}

		std::cout << "" << std::endl;

		if(file.bad() || file.fail()) throw DataException();
	}

	// Fill Data class category vectors
	for(int i = 0; i < nClasses; i++)
	{
		switch (classCategories[i])
		{
		case 1:
			mandatoryClasses.push_back(i);
			break;
		
		case 2:
			optionalClasses.push_back(i);
			break;

		case 3:
			mandatoryClasses.push_back(i);
			finalClasses.push_back(i);
			break;

		case 4:
			mandatoryClasses.push_back(i);
			preFinalClasses.push_back(i);
			break;

		default:
			throw DataException();
			break;
		}
	}

	corequisite = std::vector<std::vector<int>>(nClasses, std::vector<int>(nClasses, 0));
	prerequisite = std::vector<std::vector<int>>(nClasses, std::vector<int>(nClasses, 0));

	// Fill Data corequisite vector
	for(int k = 0; k < coreqPairs.size(); k++)
	{
		int i = coreqPairs[k].i;
		int j = coreqPairs[k].j;
		
		corequisite[i][j] = 1;
	}

	// Fill Data prerequisite vector
	for(int k = 0; k < prereqPairs.size(); k++)
	{
		int i = prereqPairs[k].i;
		int j = prereqPairs[k].j;

		prerequisite[i][j] = 1;
	}

	scheduleConflict = std::vector<std::vector<int>>(nClasses, std::vector<int>(nClasses, 0));

	// Fill schedule conflict vector
	for(int i = 0; i < classSchedules.size(); i++)
	{
		for(int j = 0; j < classSchedules.size(); j++)
		{
			if(i == j){
				scheduleConflict[i][j] = 0;
				continue;
			} 
			
			if(classSchedules[i].hasConflict(classSchedules[j])) scheduleConflict[i][j] = 1;
		}
	}

}

std::vector<int> Data::getClasses() 
{
	std::vector<int> classes;
	for(int i = 0; i < nClasses; i++)
		classes.push_back(i);
	return classes;
}

std::vector<int> Data::getSemesters()
{
	std::vector<int> semesters;
	for(int i = 1; i <= maxSemesters; i++)
		semesters.push_back(i);
	return semesters;
}
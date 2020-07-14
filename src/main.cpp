#include <iostream>
#include <string>
#include <vector>
#include <ilcplex/ilocplex.h>
#include "../include/Data.h"

using namespace std;

void solve(Data &data);

int main (int argc, char** argv)
{
	if(argc != 2)
	{
		cout << "Invalid number of arguments" << endl
			 << "usage: ./fgp [instance filepath]" << endl;
	 	return 2;
	}
	
    try{
		cout << "Loading data..." << endl;
        Data data = Data(argv[1]);
		
		try{
        	solve(data);
		}catch(IloException& e){
			cout << e.getMessage();
			e.end();
			return 2;
		}
	
	}catch(DataException e){
		cout << "Issue reading data" << endl;
        e.what();
        return 1;
    }

    return 0;
}

void solve(Data &data)
{
	IloEnv env;
	IloModel model(env);
	char name[128];


	/// Sets

	vector<int> semesters = data.getSemesters();
	vector<int> classes = data.getClasses();
	vector<int> optionalClasses = data.getOptClasses();
	vector<int> mandatoryClasses = data.getManClasses();
	vector<int> finalClasses = data.getFinClasses(); // must be taken on last semester
	vector<int> preFinalClasses = data.getPreFinClasses(); // must be taken on second to last semester


	/// Variables
	
	// X_i_k - Whether class i is taken in semester k
	IloArray<IloBoolVarArray> x(env, data.getNClasses());
	for(int i : classes)
	{
		IloBoolVarArray auxVec(env, data.getNClasses());
		x[i] = auxVec;
	}

	for(int i : classes)
	{
		for(int k : semesters)
		{
			sprintf(name, "X(%d,%d)", i, k);
			x[i][k].setName(name);
			model.add(x[i][k]);		
		}
	}

	// Y_k - Whether there are any classes in semester k
	IloBoolVarArray y(env, data.getNClasses());
	for(int k : semesters)
	{
		sprintf(name, "Y(%d)", k);
		y[k].setName(name);
		model.add(y[k]);
	}

	// Z_i_j_k - Whether classes i and j are in the same semester k
	IloArray<IloArray<IloBoolVarArray>> z(env, data.getNClasses());
	for(int i : classes)
	{
		IloArray<IloBoolVarArray> aux1(env, data.getNClasses());
		z[i] = aux1;

		for(int j = 0; j < data.getNClasses(); j++)
		{
			IloBoolVarArray aux2(env, data.getNClasses());
			z[i][j] = aux2;
		}
	}

	for(int i : classes)
	{
		for(int j = 0; j < data.getNClasses(); j++)
		{
			for(int k : semesters)
			{
				sprintf(name, "Z(%d,%d,S:%d)", i, j, k);
				z[i][j][k].setName(name);
				model.add(z[i][j][k]);
			}	
		}
	}


	/// Objective Function
	IloExpr sum_k_KY(env);

	for(int k : semesters)
	{
		sum_k_KY += k*y[k];
	}

	model.add(IloMinimize(env, sum_k_KY));


	/// Restrictions

	// (1) - Activate Y_k if there are any classes in semester
	for(int i : classes)
	{
		for(int k : semesters)
		{
			IloRange restriction = (y[k] - x[i][k] >= 0);
			sprintf(name, "R_RelXY(%d,%d)", i, k);
			restriction.setName(name);

			model.add(restriction);
		}
	}

	// (2) - There must be at least the min semester number
	IloExpr sum_k_Y(env);

	for(int k : semesters)
	{
		sum_k_Y += y[k];
	}

	IloRange restriction2 = (sum_k_Y - data.getMinSemesters() >= 0);
	sprintf(name, "R_MinSem");
	restriction2.setName(name);

	model.add(restriction2);

	// (3) - Each mandatory class must be taken once
	for(int i : mandatoryClasses)
	{
		IloExpr sum_k_X(env);

		for(int k : semesters)
		{
			sum_k_X += x[i][k];
		}

		IloRange restriction = (sum_k_X == 1);
		sprintf(name, "R_mClasses(%d)", i);
		restriction.setName(name);

		model.add(restriction);
	}

	// (4) - Sum of class hours cannot surpass max hours in a semester
	for(int k : semesters)
	{
		IloExpr sum_i_CX(env);

		for(int i : classes)
		{
			sum_i_CX += data.getClassHours(i) * x[i][k];
		}

		IloRange restriction = (sum_i_CX - data.getMaxHours() <= 0);
		sprintf(name, "R_MaxHours(%d)", k);
		restriction.setName(name);

		model.add(restriction);
	}

	// (5) - Each optional class can only be taken once
	for(int i : optionalClasses)
	{
		IloExpr sum_k_X(env);

		for(int k : semesters)
		{
			sum_k_X += x[i][k];
		}

		IloRange restriction = (sum_k_X - 1 <= 0);
		sprintf(name, "R_oClasses(%d)", i);
		restriction.setName(name);

		model.add(restriction);
	}

	// (6) - Meet min optional classes requirement
	IloExpr sum_ki_CX(env);

	for(int k : semesters)
	{
		for(int i : optionalClasses)
		{
			sum_ki_CX += data.getClassHours(i) * x[i][k];
		}
	}

	IloRange restriction6 = (sum_ki_CX - data.getMinOptHours() >= 0);
	sprintf(name, "R_minOptHours");
	restriction6.setName(name);

	model.add(restriction6);

	// (7) - Force corequisites to be taken together
	for(int i : classes)
	{
		for(int j : classes)
		{
			for(int k : semesters)
			{
				IloRange restriction = ( ((x[i][k] + x[j][k])/2) - z[i][j][k] >= 0 );
				sprintf(name, "R_Coreq(%d,%d,S:%d)", i, j, k);
				restriction.setName(name);

				model.add(restriction);
			}	
		}
	}

	// (8) - Activate one Z_k if i and j are corequisites
	for(int i : classes)
	{
		for(int j : classes)
		{
			IloExpr sum_k_Z(env);

			for(int k : semesters)
			{
				sum_k_Z += z[i][j][k];
			}	

			IloRange restriction = (sum_k_Z - data.isCoReq(i, j) == 0);
			sprintf(name, "R_ActZ(%d,%d)", i, j);
			restriction.setName(name);

			model.add(restriction);
		}
	}

	// (9) - Class can only be taken after its prerequisites
	for(int i : classes)
	{
		for(int j : classes)
		{
			for(int k : semesters)
			{
				IloExpr sum_l_X(env);

				for(int l = 0; l < k; l++)
				{
					sum_l_X += x[i][l];
				}

				IloRange restriction = ( sum_l_X - (data.isPreReq(i, j)*x[j][k]) >= 0 );
				sprintf(name, "R_Prereq(%d,%d,S:%d)", i, j, k);
				restriction.setName(name);

				model.add(restriction);
			}	
		}
	}

	// (10) - Final classes must be taken on last semester
	for(int i : finalClasses)
	{
		IloExpr sum_k_Y(env);
		IloExpr sum_k_KX(env);

		for(int k : semesters)
		{
			sum_k_Y += y[k];
			sum_k_KX += k*x[i][k];
		}

		IloRange restriction = (sum_k_Y - sum_k_KX <= 0);
		sprintf(name, "R_fClasses(%d)", i);
		restriction.setName(name);

		model.add(restriction);
	}

	// (11) - Pre final classes must be taken on second to last semester
	for(int i : preFinalClasses)
	{
		IloExpr sum_k_Y(env);
		IloExpr sum_k_KX(env);

		for(int k : semesters)
		{
			sum_k_Y += y[k];
			sum_k_KX += k*x[i][k];
		}

		IloRange restriction = ((sum_k_Y - 1) - sum_k_KX <= 0);
		sprintf(name, "R_pfClasses(%d)", i);
		restriction.setName(name);

		model.add(restriction);
	}

	// (13) - Handles schedule conflict
	for(int i : classes)
	{
		for(int j : classes)
		{
			for(int k : semesters)
			{
				IloRange restriction = (data.isScheduleConflict(i,j) * (x[i][k] + x[j][k]) - 1 <= 0);
				sprintf(name, "R_SchConflict(%d,%d,S:%d)", i, j, k);
				restriction.setName(name);

				model.add(restriction);
			}
		}
	}


	/// Solve
	const double tol = 0.1;

	IloCplex fgp(model);
	fgp.setParam(IloCplex::TiLim, 1*60*60);
	fgp.setParam(IloCplex::Threads, 1);

	fgp.exportModel("model.lp");

	try{
		fgp.solve();
	}catch(IloException& e){
		cout << e;
	}

	cout << "status: " << fgp.getStatus() << endl;
	cout << "value: " << fgp.getObjValue() << endl;

	int totalNSemesters = 0;
	for (int k : semesters)
		totalNSemesters += fgp.getValue(y[k]);
	cout << "number of semesters: " << totalNSemesters << endl;
	cout << endl;

	for(int k : semesters)
	{
		for(int i : classes)
		{
			if(fgp.getValue(x[i][k]) >= 1 - tol)
				cout << "class " << i << " in semester " << k << endl;
		}
	}
	cout << endl;
}
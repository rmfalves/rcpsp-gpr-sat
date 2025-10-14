/****************************************************************************************[Solver.C]
RCPSP-GPR SAT -- Copyright (c) 2011, Rui Alves

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/
#ifndef CNF
#define CNF

using namespace std;

#include <vector>
#include <cstring>
#include <assert.h>
#include <stdlib.h>

#include "Solver.h"

#define	AND 0
#define	OR  1

#define TMP_CNF_FILE "project_planning_tmp.cnf"
#define TMP_RES_FILE "project_planning_tmp.res"
#define SOLVER "./minisat"

#define BUFFER_SIZE 60000

class CnfFormula
{
	int numVars,numClauses;
	vector<vector<int> > clause;

	int loadResult(char *filename,vector<int> &result)
	{
		char buffer[BUFFER_SIZE];
		int i = 0,lit,start = 0,stop;
		FILE *f = fopen(filename,"rt");
		assert(f);
		fgets(buffer,BUFFER_SIZE,f);
		if (!strncmp(buffer,"UNSAT",5))
			return 0;
		assert(!strncmp(buffer,"SAT",3));

		fgets(buffer,BUFFER_SIZE,f);
		result.clear();
		while (buffer[start] != '0') {
			stop = start;
			while (buffer[stop] != ' ') {
				assert((buffer[stop] >= '0' && buffer[stop] <= '9') || (buffer[stop] == '-'));
				stop++;}
			buffer[stop] = 0;
			lit = atoi(buffer + start);
			assert((lit >= 0 ? lit : -lit) == ++i);
			if (lit != 0) {
				result.push_back(lit > 0 ? 1 : 0);
				start = stop + 1; }}
		fclose(f);
		return result.size();	
	}
		

	public:

	CnfFormula() {numVars = numClauses = 0;}

	~CnfFormula() {for (int i = 0; i < numClauses; i++) clause[i].clear(); clause.clear();}

	void fillClause(vector<int> &v,int startValue,int num,int step) 
	{
		v.resize(num);
		for (int i = 0, value = startValue; i < num; i++, value += step)
			v[i] = value;
	}
	void addClause(vector<int> &cl)
	{
		clause.resize(++numClauses);
		for (unsigned i = 0; i < cl.size(); i++) {
			clause[numClauses - 1].push_back(cl[i]);
			int x = cl[i] < 0 ? -cl[i] : cl[i];
			if (x > numVars)
				numVars = x;}
	}

	void addUnitClause(int x)
	{
		vector<int> cl;
		cl.push_back(x);
		addClause(cl);
	}

	void addBinaryClause(int x,int y)
	{
		vector<int> cl;
		cl.push_back(x);
		cl.push_back(y);
		addClause(cl);
	}

	void addTernaryClause(int x,int y,int z)
	{
		vector<int> cl;
		cl.push_back(x);
		cl.push_back(y);
		cl.push_back(z);
		addClause(cl);
	}

	void saveFormula(const char *filename)
	{
		FILE *f;
		if (*filename == 0)
			f = stdout;
		else	
			f = fopen(filename,"wt");
		fprintf(f,"p cnf %d %d\n",numVars,numClauses);
		for (int i = 0; i < numClauses; i++) {
			for (unsigned j = 0; j < clause[i].size(); j++)
				fprintf(f,"%d ",clause[i][j]);
			fprintf(f,"0\n"); }
		if (f != stdout)
			fclose(f);
	}

  int solve(vector<int> &result)
  {
    Solver solver;
    vec<Lit> lits;
    for (int i = 0; i < clause.size(); i++) {
      lits.clear();
      for (int j = 0; j < clause[i].size(); j++) {
        int var = abs(clause[i][j]) - 1;
        while (var >= solver.nVars()) solver.newVar();
        lits.push((clause[i][j] > 0) ? Lit(var) : ~Lit(var)); }
      solver.addClause(lits); }
    solver.verbosity = 0;
    //saveFormula("formula.cnf");
    if (solver.solve()) {    
      result.resize(solver.nVars());
      for (int i = 0; i < solver.nVars(); i++) {
        if (solver.model[i] == l_True)
          result[i] = 1;
        else if (solver.model[i] == l_False)
          result[i] = 0;
        else
          result[i] = -1;}
      return 1;}
    else
      return 0;
  }

	/* Implications Section */

	void addImplication(int x,int y) {addBinaryClause(-x,y);}

	void addImplication(int x,int op,vector<int> &y)
	{
		if (op == OR) {
			vector<int> v;
			v.push_back(-x);
			for (unsigned i = 0; i < y.size(); i++)
				v.push_back(y[i]);
			addClause(v);}
		else {
			for (unsigned i = 0; i < y.size(); i++)
				addBinaryClause(-x,y[i]);}
	}

	void addImplication(int op,vector<int> &x,int y)
	{
		if (op == AND) {
			vector<int> v;
			for (unsigned i = 0; i < x.size(); i++)
				v.push_back(-x[i]);
			v.push_back(y);
			addClause(v);}
		else {
			for (unsigned i = 0; i < x.size(); i++)
				addBinaryClause(-x[i],y);}
	}

	void addImplication(int op1,vector<int> &x,int op2,vector<int> &y)
	{
		vector<int> v;
		if (op1 == AND && op2 == OR) {
			for (unsigned i = 0; i < x.size(); i++)
				v.push_back(-x[i]);
			for (unsigned i = 0; i < y.size(); i++)
				v.push_back(y[i]);
			addClause(v);}
		else if (op1 == AND && op2 == AND) {
			for (unsigned i = 0; i < x.size(); i++)
				v.push_back(-x[i]);
			v.push_back(0);
			int top = v.size() - 1;
			for (unsigned i = 0; i < y.size(); i++) {
				v[top] = y[i];
				addClause(v);}}
		else if (op1 == OR && op2 == OR) {
			v.push_back(0);
			for (unsigned i = 0; i < y.size(); i++)
				v.push_back(y[i]);
			for (unsigned i = 0; i < x.size(); i++) {
				v[0] = -x[i];
				addClause(v);}}
		else {
			for (unsigned i = 0; i < x.size(); i++)
				for (unsigned j = 0; j < y.size(); j++)
					addBinaryClause(-x[i],y[j]);}
	}


	/* Equivalences Section */

	void addEquivalence(int x,int y) {addImplication(x,y); addImplication(y,x);}

	void addEquivalence(int x,int op,vector<int> &y) {addImplication(x,op,y); addImplication(op,y,x);}

	void addEquivalence(int op,vector<int> &x,int y) {addImplication(op,x,y);addImplication(y,op,x);}

	void addEquivalence(int op1,vector<int> &x,int op2,vector<int> &y) {addImplication(op1,x,op2,y); addImplication(op2,y,op1,x);}

	
	/* XOR */
	
	void addXorClause(vector<int> &x)
	{
		vector<int> right;
		addClause(x);
		for (unsigned i = 1; i < x.size(); i++)
			right.push_back(-x[i]);
		for (unsigned i = 0, sz = x.size() - 1; i < sz; i++)
			for (unsigned j = i + 1; j < x.size(); j++)
				addBinaryClause(-x[i],-x[j]);
	}
	

};

#endif

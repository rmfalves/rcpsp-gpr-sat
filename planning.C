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
#ifndef PLANNING
#define PLANNING

#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include "satvar.C"
#include "cnf.C"
#include "graph.C"
#include "resources.C"

#define FALSE 0
#define TRUE  1

using namespace std;

class ProjectPlanning
{
  public:
  
  typedef enum {SS,SF,FS,FF} SequenceType;

  struct Activity
  {
	int earliest_start,latest_start,earliest_finish,latest_finish;
	int duration,set_start;
	int start,finish; // Optimize at verification time
	vector<int> successor,predecessor; 

	Activity() {duration = -1; set_start = -1; earliest_start = latest_start = earliest_finish = latest_finish = 0;}
  };

  struct ActivitySequence
  {
    int activity1,activity2;
  	SequenceType sequence;

	  ActivitySequence(int activity1,int activity2,SequenceType sequence)
  	{
	  	this->activity1 = activity1;
		  this->activity2 = activity2;
  		this->sequence = sequence;
	  }

  };
  
	int times,activities,resources;
	CnfFormula cnf;
	SatMatrix time_act,time_act_st,time_act_fi;
  vector<vector<int> > act_res;
  vector<int> availability;
	vector<Activity> activity;
	vector<ActivitySequence> activity_sequence;

  void verifyUniqueStart();
  void verifyUniqueFinish();
  void verifyLatestStart();
  void verifyEarliestFinish();
  void verifyStartAndFinishInterval();
  void verifyTimesInIntervalAndNoTimesGap();
  void verifyActivitySequencing();
  void verifyPreScheduledActivities();
  void verifyResourcesAvailability();

  public:

  void verify();
    

	void Build(int num_activities,int time_interval,int num_resources)
	{
		int base_index = 1;
		time_act.Build(base_index,num_activities,time_interval,'T','A',"####");

		base_index += num_activities * time_interval;
		time_act_st.Build(base_index,num_activities,time_interval,'T','A'," X  ");
		base_index += num_activities * time_interval;
		time_act_fi.Build(base_index,num_activities,time_interval,'T','A'," X  ");
		base_index += num_activities * time_interval;
    act_res.resize(num_resources);
    for (unsigned res = 0; res < num_resources; res++) {
      act_res[res].resize(num_activities);
      for (unsigned act = 0 ; act < num_activities; act++)
        act_res[res][act] = 0;
    }
    availability.resize(num_resources);    
    for (unsigned res = 0 ; res < num_resources; res++)
      availability[res] = 1;
    for (unsigned act = 0; act < num_activities; act++)
      activity.push_back(Activity());
		resources = num_resources;
		activities = num_activities;
		times = time_interval;
	}

	~ProjectPlanning()
	{
		activity.clear();
		activity_sequence.clear();
		for (unsigned res = 0; res < act_res.size(); res++)
			act_res[res].clear();
		act_res.clear();
		availability.clear();
	}


  void defineActivity(int act,int duration,int start)
  {
    activity[act].duration = duration;
    activity[act].set_start = start;
  }

  void defineActivitySequence(int act1,int act2,SequenceType seq)
  {
    	activity_sequence.push_back(ActivitySequence(act1,act2,seq));
	// Necessary for the critical path purposes, will be discarded later
	if (seq == FS) {
		activity[act1].successor.push_back(act2);
		activity[act2].predecessor.push_back(act1);
	}
  }

  bool checkActivities()
  {
    for (int i = 0; i < activity.size(); i++)
      if (activity[i].duration == -1)
        return false;
    return true;
  }
        
  void setResourceAvailability(int resource,int amount)
  {
    availability[resource] = amount;
  }

  void allocResource(int activity,int resource,int amount)
  {
    act_res[resource][activity] = amount;
  }


  /***************************** Formulation ***********************************/
     
  void uniqueStart()
  {
    vector<int> cl;
    for (int act = 0, offset = time_act_st.base_index; act < time_act_st.rows; act++, offset += time_act_st.columns) {
      cnf.fillClause(cl,offset,time_act_st.columns,1);
      cnf.addXorClause(cl);}
  }

  void uniqueFinish()
  {
    vector<int> cl;
    for (int act = 0, offset = time_act_fi.base_index; act < time_act_fi.rows; act++, offset += time_act_fi.columns) {
      cnf.fillClause(cl,offset,time_act_fi.columns,1);
      cnf.addXorClause(cl);}
  }

  void latestStart()
  {
    for (int act = 0; act < time_act.rows; act++) {

      activity[act].latest_start = time_act.columns - activity[act].duration + 1;
      for (int t = activity[act].latest_start; t < time_act.columns; t++)
        cnf.addUnitClause(-time_act_st.cnfVar(act,t));}
  }
    

  void earliestFinish()
  {
    for (int act = 0; act < time_act.rows; act++) {
      activity[act].earliest_finish = activity[act].duration - 1;
      for (int t = 0; t < activity[act].earliest_finish; t++)
        cnf.addUnitClause(-time_act_fi.cnfVar(act,t));}
  }

  void startAndFinishInterval()
  { 
    for (int act = 0; act < time_act.rows; act++) {
      for (int t = 0; t < activity[act].latest_start; t++)
        cnf.addImplication(time_act_st.cnfVar(act,t),time_act_fi.cnfVar(act,t + activity[act].duration - 1));}
  }

  void noTimeGaps()
  {
    vector<int> time1,time2;
    for (int t = 0; t < time_act.columns - 1; t++) {
      cnf.fillClause(time1,time_act.cnfVar(0,t),time_act.rows,time_act.columns);

      cnf.fillClause(time2,time_act.cnfVar(0,t + 1),time_act.rows,time_act.columns);
      for (int i = 0; i < time_act.rows; i++) {
        time1[i] = -time1[i];
        time2[i] = -time2[i];}
      cnf.addImplication(AND,time1,AND,time2);}
  }

  void setTimesInInterval()
  {
    vector<int> pattern;
    pattern.resize(time_act.columns);
    for (int act = 0; act < time_act.rows; act++) {
      for (int i = 0; i < activity[act].duration; i++)
        pattern[i] = time_act.cnfVar(act,i);
      for (int i = activity[act].duration; i < time_act.columns; i++)
        pattern[i] = -time_act.cnfVar(act,i);
      for (int start = 0; start < activity[act].latest_start; start++) {
        cnf.addImplication(time_act_st.cnfVar(act,start),AND,pattern);
        if (start < activity[act].latest_start - 1) {
          pattern[start] = -pattern[start];
          pattern[start + activity[act].duration] = -pattern[start + activity[act].duration];}}}
  }
          
                
	void startToStart(int act1,int act2)
	{
    vector<int> pattern;
    for (int t1 = 0; t1 <= activity[act1].latest_start; t1++) {
      pattern.clear();
      for (int t2 = 0; t2 <= t1; t2++)
        pattern.push_back(-time_act_st.cnfVar(act2,t2));
      cnf.addImplication(time_act_st.cnfVar(act1,t1),AND,pattern);}
	}

	void startToFinish(int act1,int act2)
	{
    vector<int> pattern;
    for (int t1 = 0; t1 <= activity[act1].latest_start; t1++) {
      pattern.clear();
      for (int t2 = 0; t2 <= t1; t2++)
        pattern.push_back(-time_act_fi.cnfVar(act2,t2));
      cnf.addImplication(time_act_st.cnfVar(act1,t1),AND,pattern);}
	}


	void finishToStart(int act1,int act2)
	{
		vector<int> pattern;
		for (int t1 = activity[act1].earliest_finish; t1 < time_act_fi.columns; t1++) {
			pattern.clear();
			for (int t2 = 0; t2 <= t1; t2++)
				pattern.push_back(-time_act_st.cnfVar(act2,t2));
			cnf.addImplication(time_act_fi.cnfVar(act1,t1),AND,pattern);
		}
	}

  
	void finishToFinish(int act1,int act2)
	{
    vector<int> pattern;
    for (int t1 = activity[act1].earliest_finish; t1 < time_act_fi.columns; t1++) {
      pattern.clear();
      for (int t2 = 0; t2 <= t1; t2++)
        pattern.push_back(-time_act_fi.cnfVar(act2,t2));
      cnf.addImplication(time_act_fi.cnfVar(act1,t1),AND,pattern);}
	}

	void activitySequencing()
	{
		for (unsigned seq = 0; seq < activity_sequence.size(); seq++)
			switch (activity_sequence[seq].sequence) {
				case SS: startToStart(activity_sequence[seq].activity1,activity_sequence[seq].activity2); break;
				case SF: startToFinish(activity_sequence[seq].activity1,activity_sequence[seq].activity2); break;
				case FS: finishToStart(activity_sequence[seq].activity1,activity_sequence[seq].activity2); break;
				case FF: finishToFinish(activity_sequence[seq].activity1,activity_sequence[seq].activity2); break;}
	}


	void preScheduledActivities()
	{
		for (unsigned act = 0; act < activities; act++)
      if (activity[act].set_start != -1)
        cnf.addUnitClause(time_act_st.cnfVar(act,activity[act].set_start));
	}

	void resourceAvailabilities()
	{
		Resource r;
		vector<int> pattern;
		vec<int> conflict;

		for (int res = 0; res < resources; res++)
			r.constrain(activities,&act_res[res][0],availability[res]);

		for (int conf = 0; conf < r.conflicts.size(); conf++) {
			r.decode_from_pattern(r.conflicts[conf],conflict);
			pattern.resize(conflict.size());
			for (int time = 0; time < times; time++) {
				for (int i = 0; i < conflict.size(); i++)	
				pattern[i] = -time_act.cnfVar(conflict[i],time);
				cnf.addClause(pattern);
			}
		}
	}

	void connectStartActivity()
	{
		for (int act = 1; act < activities - 1; act++)
			if (activity[act].predecessor.size() == 0)
				defineActivitySequence(0,act,FS);
	}

	void connectTerminalActivity()
	{
		for (int act = 1; act < activities - 1; act++)
			if (activity[act].successor.size() == 0)
				defineActivitySequence(act,activities - 1,FS);
	}

	void criticalPath()
	// Set earliest start time and latest finish time for every activity.
	{
		//Create temporary start and finish activities
		activity.push_back(Activity());
		for (int act = activity.size() - 1; act > 0; act--)
			activity[act] = activity[act - 1];
		activity[0].duration = 0;
		activity[0].predecessor.clear();
		activity[0].successor.clear();
		activity.push_back(Activity());
		activity[activity.size() - 1].duration = 0; 
		activities += 2;

		// Shift references by one, including sequence lists 
		for (int seq = 0; seq < activity_sequence.size(); seq++) {
			activity_sequence[seq].activity1++;
			activity_sequence[seq].activity2++;
		}
		for (int act = 0; act < activities; act++) {
			for (int i = 0; i < activity[act].successor.size(); i++)
				activity[act].successor[i]++;
			for (int i = 0; i < activity[act].predecessor.size(); i++)
				activity[act].predecessor[i]++;
		}

		int previous_seq_size = activity_sequence.size();
		connectStartActivity();
		connectTerminalActivity();

		// Walk list ahead
		activity[0].earliest_start = 0;
		activity[0].earliest_finish = activity[0].earliest_start + activity[0].duration;

		for (int i = 0; i < activities; i++) {
			for (int j = 0; j < activity[i].predecessor.size(); j++) {
				int pred = activity[i].predecessor[j];
				if (activity[i].earliest_start < activity[pred].earliest_finish)
					activity[i].earliest_start = activity[pred].earliest_finish;
				activity[i].earliest_finish = activity[i].earliest_start + activity[i].duration;
			}}

		// Walk list back
		activity[activities - 1].latest_finish = times;
		activity[activities - 1].latest_start = activity[activities - 1].latest_finish - activity[activities - 1].duration;
		for (int i = activities - 2; i >= 0; i--) {
			for (int j = 0; j < activity[i].successor.size(); j++) {
				int succ = activity[i].successor[j];
				if (activity[i].latest_finish == 0)
					activity[i].latest_finish = activity[succ].latest_start;
				else if (activity[i].latest_finish > activity[succ].latest_start)
					activity[i].latest_finish = activity[succ].latest_start;
			}
			activity[i].latest_start = activity[i].latest_finish - activity[i].duration;
		}

		// Discard temporary start and finish activities, clear sequence lists
		for (int act = 0; act < activity.size() - 2; act++) {
			activity[act] = activity[act + 1];
			activity[act].successor.clear();
			activity[act].predecessor.clear();
		}
		
		// Restore previous references, and remove sequences with the former start and finish activities
		// Removing those sequences is made easy because they are always the last ones appended to the vector 
		for (int seq = activity_sequence.size(); seq > previous_seq_size; seq--) 
			activity_sequence.pop_back();
		for (int seq = 0; seq < activity_sequence.size(); seq++) {
			activity_sequence[seq].activity1--;
			activity_sequence[seq].activity2--;
		}
		
		// Restore vector previous state
		activity.pop_back();
		activity.pop_back();
		activities -= 2;

		// Adjust time references
		for (int act = 0; act < activities; act++) {
			activity[act].earliest_start--;
			activity[act].earliest_finish -= 2;
			activity[act].latest_finish--;
		}
	}

	void disableNonCandidates()
	{
		criticalPath();
		for (int act = 0; act < activities; act++) {
			for (int t = 0; t < activity[act].earliest_start; t++) {
				cnf.addUnitClause(-time_act_st.cnfVar(act,t));
				cnf.addUnitClause(-time_act.cnfVar(act,t));
			}
			for (int t = activity[act].latest_start + 1; t < times; t++) {
				cnf.addUnitClause(-time_act_st.cnfVar(act,t));
			}
			for (int t = 0; t < activity[act].earliest_finish; t++) {
				cnf.addUnitClause(-time_act_fi.cnfVar(act,t));
			}
			for (int t = activity[act].latest_finish + 1; t < times; t++) {
				cnf.addUnitClause(-time_act_fi.cnfVar(act,t));
				cnf.addUnitClause(-time_act.cnfVar(act,t));
			}
		}
	}

	void buildFormula()
	{
		uniqueStart();
		uniqueFinish();
		latestStart();
		earliestFinish();		
		startAndFinishInterval();
		noTimeGaps();
		setTimesInInterval();
		activitySequencing();
		preScheduledActivities();
		resourceAvailabilities();
		disableNonCandidates();
	}

	/*****************************************************************************/

  void dump()
  {
    printf("\n==============================================================================\n");

    printf("Activities Duration:\n");
    for (unsigned i = 0; i < activities; i++)
      printf("(%d --> %d) ",i,activity[i].duration);
    printf("\n\nActivities Sequence:\n");
    for (unsigned i = 0; i < activity_sequence.size(); i++)
      printf("(%d,%d,%d) ",activity_sequence[i].activity1,activity_sequence[i].activity2,activity_sequence[i].sequence);
    printf("\n==============================================================================\n");
  }
      
	void report(FILE *f)
	{
		fprintf(f,"\n");
		//time_act_st.report(f);
		//time_act_fi.report(f);
		time_act.report(f);
	}

	bool solve(char *report_file)
	{
		vector<int> result;
		FILE *f = report_file ? fopen(report_file,"wt") : stdout;

		if (!f) return false;

		buildFormula();
		kill(getpid(),SIGUSR1);
		if (!cnf.solve(result)) {
			fprintf(f,"No solution found.\n\n");}
		else {
			time_act.loadFromList(result);
			time_act_st.loadFromList(result);
			time_act_fi.loadFromList(result);

			verify();
			report(f);
		}
		if (f != stdout) fclose(f);
		return true;
	}

  ActivitySequence *buildSequenceCycle(int *path)
  {
    ActivitySequence *buffer = (ActivitySequence *)malloc((activities + 1) * sizeof(ActivitySequence));
    int size = 0;
    for (int i = 1; path[i] != -1; i++) {
      int act1 = path[i - 1] / 2;
      int act2 = path[i] / 2;
      if (act1 == act2) continue;
      buffer[size].activity1 = act1; 
      buffer[size].activity2 = act2; 
      buffer[size++].sequence = (SequenceType)((path[i - 1] & 1) * 2 + (path[i] & 1));}

    buffer[size].activity1 = -1; 
    buffer[size++].activity2 = -1;
    return (ActivitySequence *)realloc(buffer,size * sizeof(ActivitySequence)); 

  }

  ActivitySequence *checkSequences()
  {
    int *cycle;
    graph g(activities * 2);
    for (int i = 0,sz = activities << 1; i < sz; i += 2)
      g.newEdge(i,i + 1);
    for (int i = 0; i < activity_sequence.size(); i++) {
      int lin = (activity_sequence[i].activity1 << 1) + (activity_sequence[i].sequence >> 1); 
      int col = (activity_sequence[i].activity2 << 1) + (activity_sequence[i].sequence & 1); 
      g.newEdge(lin,col);}
    cycle = g.hasCycle();
    return cycle == NULL ? NULL : buildSequenceCycle(cycle);
  }

};

#endif

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
#ifndef VERIFY
#define VERIFY

#include <stdio.h>
#include "planning.C"

void ProjectPlanning::verify()
{
    verifyUniqueStart();
    verifyUniqueFinish();
    verifyLatestStart();
    verifyEarliestFinish();
    verifyStartAndFinishInterval();
    verifyTimesInIntervalAndNoTimesGap();
    verifyActivitySequencing();
    verifyResourcesAvailability();
    verifyPreScheduledActivities();
    fprintf(stderr,"Verifies ok\n");
}


void ProjectPlanning::verifyUniqueStart()
{
    for (int act = 0; act < activities; act++) {
        int count = 0;
        for (int t = 0; t < times; t++)
            if (time_act_st.getElem(act,t) == 1) {
                count++;
                assert(count <= 1);}
        assert(count > 0);}
}


void ProjectPlanning::verifyUniqueFinish()
{
    for (int act = 0; act < activities; act++) {
        int count = 0;
        for (int t = 0; t < times; t++)
            if (time_act_fi.getElem(act,t) == 1) {
                count++;
                assert(count <= 1);}
        assert(count > 0);}
}


void ProjectPlanning::verifyLatestStart()
{
    for (int act = 0; act < time_act.rows; act++)
        for (int t = time_act_st.columns - activity[act].duration + 1; t < time_act.columns; t++)
            assert(time_act_st.getElem(act,t) == 0);
}


void ProjectPlanning::verifyEarliestFinish()
{
    for (int act = 0; act < time_act.rows; act++)
        for (int t = 0,sz = activity[act].duration - 1; t < sz; t++)
            assert(time_act_fi.getElem(act,t) == 0);
}


void ProjectPlanning::verifyStartAndFinishInterval()
{
    for (int act = 0; act < time_act.rows; act++)
        for (int t = 0; t < time_act_st.columns; t++) {
            int finish = t + activity[act].duration - 1;
            assert(time_act_st.getElem(act,t) == 0 || time_act_fi.getElem(act,finish) == 1); }
}


void ProjectPlanning::verifyTimesInIntervalAndNoTimesGap()
{
    for (int act = 0; act < time_act.rows; act++) {
        int t = 0;
        for ( ; time_act.getElem(act,t) == 0; t++);
        assert(t < time_act.columns);
        activity[act].start = t;
        for (int end_time = t + activity[act].duration; t < end_time; t++)
            assert(time_act.getElem(act,t) == 1);
        assert(t <= time_act.columns);
        activity[act].finish = t - 1;
        while (t < time_act.columns)
            assert(time_act.getElem(act,t++) == 0);}
}


void ProjectPlanning::verifyActivitySequencing()
{
    for (int seq = 0,sz = activity_sequence.size(); seq < sz; seq++) {
        int a1 = activity_sequence[seq].activity1;
        int a2 = activity_sequence[seq].activity2;
        assert(activity_sequence[seq].sequence != SS || activity[a1].start < activity[a2].start);
        assert(activity_sequence[seq].sequence != SF || activity[a1].start < activity[a2].finish);
        assert(activity_sequence[seq].sequence != FS || activity[a1].finish < activity[a2].start);
        assert(activity_sequence[seq].sequence != FF || activity[a1].finish < activity[a2].finish);}
}


void ProjectPlanning::verifyPreScheduledActivities()
{
    for (int act = 0; act < time_act.rows; act++) {
      assert(activity[act].set_start == -1 || activity[act].set_start == activity[act].start);
    }
}


void ProjectPlanning::verifyResourcesAvailability()
{
    for (int res = 0; res < resources; res++)
      for (int time = 0; time < times; time++) {
        int sum = 0;
        for (int act = 0; act < activities; act++)
          if (time_act.getElem(act,time) == 1)
            sum += act_res[res][act];
        assert(sum <= availability[res]);
      }
}



#endif

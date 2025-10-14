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
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "Solver.h"
#include "pdsl.C"

const int DOT_INTERVAL = 10;

static inline double cpuTime(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000; }

static inline int memReadStat(int field)
{
    char    name[256];
    pid_t pid = getpid();
    sprintf(name, "/proc/%d/statm", pid);
    FILE*   in = fopen(name, "rb");
    if (in == NULL) return 0;
    int     value;
    for (; field >= 0; field--)
        fscanf(in, "%d", &value);
    fclose(in);
    return value;
}

static inline uint64_t memUsed() { return (uint64_t)memReadStat(0) * (uint64_t)getpagesize(); }

void help(char *exec_name)
{
    fprintf(stderr,"Program Usage:\n",exec_name);
    fprintf(stderr,"%s [input_file] [-r <report_file>]\n",exec_name);
    fprintf(stderr,"%s [-h] | [-?]\n\n",exec_name);
}

static void SIGALRM_handler(int signum)
{
  fprintf(stderr,".");
  alarm(DOT_INTERVAL);
}

double solver_time = 0;
double previous_mem = 0;


static void SIGUSR1_handler(int signum)
{
	puts("Solver has entered...");
	solver_time = cpuTime();
	previous_mem = memUsed();
}



int main(int argc, char** argv)
{
    if ((argc == 2) && (!strcmp(argv[1],"-h") || !strcmp(argv[1],"-?"))) {
      help(argv[0]);
      return 0;}

    int rep_switch_pos;
    for (rep_switch_pos = 1; rep_switch_pos < argc; rep_switch_pos++)
      if (!strcmp(argv[rep_switch_pos],"-r")) break;
    if (rep_switch_pos == argc - 1) {
      fprintf(stderr,"Error: missing report file.\n");
      return(1);}

    bool rep_file = (rep_switch_pos < argc);
    if (!rep_file && argc > 2) {
      fprintf(stderr,"Error: invalid argument.\n");
      return(1);}
            
    fprintf(stderr,"PDSL - Project Definition Scripting Language.\n");
    char *input_file_name = NULL,*report_file_name = NULL;
    if ((argc == 1) || (rep_switch_pos == 1))
      fprintf(stderr,"Reading from standard input.\n");
    else
      input_file_name = strdup(argv[1]);
    if (rep_file)
      report_file_name = strdup(argv[rep_switch_pos + 1]);
      
    signal(SIGALRM,SIGALRM_handler);
    signal(SIGUSR1,SIGUSR1_handler);
    //alarm(DOT_INTERVAL);
    double total_time = cpuTime();
    Pdsl pdsl;

    fprintf(stderr,"Solving...\n");
    pdsl.run(input_file_name,report_file_name);
    fprintf(stderr,"\n Total Time: %0.3f second\t\t Total Memory Usage: %0.3f Mbyte",cpuTime() - total_time,memUsed() / 1048576.0);
    fprintf(stderr,"\nSolver Time: %0.3f second\t\tSolver Memory Usage: %0.3f Mbyte\n\n",cpuTime() - solver_time,(memUsed() - previous_mem) / 1048576.0);
}

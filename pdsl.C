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
#ifndef PDSL
#define PDSL

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include "planning.C"

using namespace std;

const char  COMMENT = '#';
const int   MAX_ARG_SIZE = 32;
const int   MAX_LINE_SIZE = 256;

class Pdsl
{
  vector<char *> pdslKeyword;

  char error_msg[11][64];
  int error_msg_id;
  
  int activities,times,resources;
  int current_line;
  ProjectPlanning project;

  void loadPdslKeywords()
  {
    pdslKeyword.push_back("PROJECT");
    pdslKeyword.push_back("ACTIVITY");
    pdslKeyword.push_back("SEQUENCE");
    pdslKeyword.push_back("RESOURCE");
    pdslKeyword.push_back("ALLOCATE");
  }

  void loadErrorMessages()
  {
    strcpy(error_msg[0],"Invalid command.");
    strcpy(error_msg[1],"Wrong number of arguments.");
    strcpy(error_msg[2],"Invalid argument type.");
    strcpy(error_msg[3],"Argument out of range.");
    strcpy(error_msg[4],"Start time must not be before finish time.");
    strcpy(error_msg[5],"Wrong sequence type.");
    strcpy(error_msg[6],"Missing PROJECT statement.");
    strcpy(error_msg[7],"Duplicated PROJECT statement.");
    strcpy(error_msg[8],"There are one or more activities with no duration specified.");
    strcpy(error_msg[9],"Unable to load PDSL file.");
    strcpy(error_msg[10],"Unable to create report file.");
  }

                                                                                                                                                                                                                        
  char *skipBlanks(char *text)
  {
    char *ptr;
    for (ptr = text; isspace(*ptr); ptr++);
    return ptr;
  }

  bool isBlankLine(char *line)
  {
    line = skipBlanks(line);
    int len = strlen(line);
    if (!len) return true;
    if (line[len - 1] == '\n') line[--len] = 0;
    if (!len) return true;
    if (*line == COMMENT) return true;
    return false;
  }

  char *getWord(char *text)
  {
    char *ptr;
    for (ptr = text; !isspace(*ptr); ptr++);
    *ptr = 0;
    return(text);
  }

  char *strToUpper(char *str)
  {
    for (char *ptr = str; *ptr != 0; ptr++)
      *ptr = toupper(*ptr);
    return str;
  }

  int findKeyword(char *keyword)
  {
    strToUpper(keyword);
    error_msg_id = -1;
    for (int i = pdslKeyword.size() - 1; i >= 0; i--)
      if (!strcmp(keyword,pdslKeyword[i]))
        return i + 1;
    error_msg_id = 0;
    return -1;
  }

  int parse(char *line,vector<char *> &args)
  {
    args.clear();
    if (isBlankLine(line)) return 0;
    strcat(line," ");
    char *ptr = skipBlanks(line);
    int keyword_id = findKeyword(getWord(ptr));
    if (keyword_id == -1) return -1;
    args.push_back(ptr);
    ptr += strlen(ptr) + 1;
    while (*ptr != 0) {
      ptr = skipBlanks(ptr);
      if (*ptr) {
        args.push_back(strToUpper(getWord(ptr)));
        ptr += strlen(ptr) + 1;}}
    return keyword_id;
  }  

  bool isInteger(char *str)
  {
    for (char *ptr = str; *ptr; ptr++)
      if (!isdigit(*ptr))
        return false;
    return true;
  }

  bool isInRange(const char *number,int lower,int upper)
  {
    return atoi(number) >= lower && atoi(number) <= upper;
  }

  bool isValidSequenceType(char *str)
  {
    if (strlen(str) != 2) return false;
    strToUpper(str);
    return (str[0] == 'S' || str[0] == 'F' || str[1] == 'S' || str[1] == 'F');
  }

  bool validateArgs(int keyword_id,vector<char *> args)
  {
    switch (keyword_id - 1) {
      case 0:
        if (args.size() != 4) {
          error_msg_id = 1;
          return false;}
        for (int i = 1; i < 4; i++)
          if (!isInteger(args[i])) {
            error_msg_id = 2;
            return false;}
        for (int i = 1; i < 4; i++)
          if (!isInRange(args[i],1,50000)) {
            error_msg_id = 3;
            return false;}
        break;
      case 1:
        if (args.size() != 3 && args.size() != 4) {
          error_msg_id = 1;
          return false;}
        for (int i = 1; i < args.size(); i++)
          if (!isInteger(args[i])) {
            error_msg_id = 2;
            return false;}
        for (int i = 1; i < args.size(); i++) {
          int max;
          switch (i) {
            case 1: max = activities; break;
            case 2: max = times; break;
            case 3: max = times - atoi(args[2]) + 1; break;}
          if (!isInRange(args[i],1,max)) {
            error_msg_id = 3;
            return false;}}
        break;
      case 2:
        if (args.size() != 4) {
          error_msg_id = 1;
          return false;}
        for (int i = 1; i < 3; i++)
          if (!isInteger(args[i])) {
            error_msg_id = 2;
            return false;}
        if (!isInRange(args[1],1,activities) || !isInRange(args[2],1,activities)) {
          error_msg_id = 3;
          return false;}
        if (!isValidSequenceType(args[3])) {
          error_msg_id = 5;
          return false;}
        break;
      case 3:
        if (args.size() != 3) {
          error_msg_id = 1;
          return false;}
        for (int i = 1; i < 3; i++)
          if (!isInteger(args[i])) {
            error_msg_id = 2;
            return false;}
        if (!isInRange(args[1],1,resources) || !isInRange(args[2],1,50000)) {
          error_msg_id = 3;
          return false;}
        break;
      case 4:
        if (args.size() != 4) {
          error_msg_id = 1;
          return false;}
        for (int i = 1; i < 4; i++)
          if (!isInteger(args[i])) {
            error_msg_id = 2;
            return false;}
        if (!isInRange(args[1],1,activities) || !isInRange(args[2],1,resources) || !isInRange(args[3],1,50000)) {
          error_msg_id = 3;
          return false;}
        break;
    }
    return true;
  }

  bool validateHeader(int id)
  {
    if (id > 1 && activities < 0) {
      error_msg_id = 6;
      return false;}
    if (id == 1 && activities > 0) {
      error_msg_id = 7;
      return false;}
    return true;
  }
  
  void executeCommand(int id,vector<char *> &args,ProjectPlanning &proj)
  {
    int s,f,r,a,t,a1,a2,bit1,bit2,x;
    ProjectPlanning::SequenceType seq;
    switch (id - 1) {
      case 0:
        activities = atoi(args[1]);
        times = atoi(args[2]);
        resources = atoi(args[3]);
        proj.Build(activities,times,resources);
        break;
      case 1:
        a = atoi(args[1]) - 1;
        t = atoi(args[2]);
        proj.defineActivity(a,t,args.size() > 3 ? atoi(args[3]) - 1: -1);
        break;
      case 2:
        a1 = atoi(args[1]) - 1;
        a2 = atoi(args[2]) - 1;
        bit1 = args[3][0] == 'S' ? 0 : 2;
        bit2 = args[3][1] == 'S' ? 0 : 1;
        seq = (ProjectPlanning::SequenceType)(bit1 | bit2);
        proj.defineActivitySequence(a1,a2,seq);
        break;
      case 3:
        r = atoi(args[1]) - 1;
        x = atoi(args[2]);
        proj.setResourceAvailability(r,x);
        break;
      case 4:
        a = atoi(args[1]) - 1;
        r = atoi(args[2]) - 1;
        x = atoi(args[3]);
        proj.allocResource(a,r,x);
        break;
    }
  }

  
  public:

  Pdsl()
  {
    activities = times = resources = error_msg_id = -1;
    loadPdslKeywords();
    loadErrorMessages();    
  }
  
  bool executeCommandLine(char *line,ProjectPlanning &pp)
  {
    vector<char *> arg;
    int id = parse(line,arg);
    if (id < 0) return false;
    if (id == 0) return true;
    if (!validateHeader(id)) return false;
    if (!validateArgs(id,arg)) return false;
    executeCommand(id,arg,pp);
    return true;
  }

  bool loadPdslScript(FILE *pdslFile,ProjectPlanning &pp)
  {
    char line[MAX_LINE_SIZE];
    current_line = 1;
    do {
      fgets(line,MAX_LINE_SIZE,pdslFile);
      if (!executeCommandLine(line,pp)) {
        fprintf(stderr,"Error reading line %d: %s\n",current_line,error_msg[error_msg_id]);
        return false;}
      *line = 0;
      current_line++;}
    while (!feof(pdslFile));
    if (!pp.checkActivities()) {
      error_msg_id = 8;
      fprintf(stderr,"Error: %s\n",error_msg[error_msg_id]);
      return false;}
    return true;
  }

  bool run(char *pdslFile,char *result_file)
  {
    FILE *f;
    if (pdslFile) {
      f = fopen(pdslFile,"rt");
      if (!f) {
        error_msg_id = 9;
        fprintf(stderr,"Error: %s\n",error_msg[error_msg_id]);
        return false;}}
    else
      f = stdin;
    if (!loadPdslScript(f,project)) return false;
    ProjectPlanning::ActivitySequence *cycle_error = project.checkSequences();
    if (cycle_error != NULL) {
      fprintf(stderr,"Circular reference found:\n");
      for (int i = 0; cycle_error[i].activity1 != -1; i++)
        fprintf(stderr,"%d %c%c %d\n",cycle_error[i].activity1 + 1,cycle_error[i].sequence >> 1 == 0 ? 'S' : 'F',cycle_error[i].sequence & 1 ? 'F' : 'S',cycle_error[i].activity2 + 1);
      fprintf(stderr,"\n");
      return false;}
    if (!project.solve(result_file)) {
      error_msg_id = 10;
      fprintf(stderr,"Error: %s\n",error_msg[error_msg_id]);
      return false;}
    if (f != stdin)
      fclose(f);
    return true;
  }
  
};

#endif

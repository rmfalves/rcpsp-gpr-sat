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
#ifndef GRAPH
#define GRAPH

#include <malloc.h>
#include <stdio.h>

#define END_OF_PATH -1

class graph
{
  enum node_color {WHITE,BLACK,GREY};

  bool **adjacency;
  int nodes;


  public:

  graph(int nodes)
  {
    adjacency = (bool **)malloc(nodes * sizeof(bool *));
    for (int i = 0; i < nodes; i++)
      adjacency[i] = (bool *)malloc(nodes * sizeof(bool));
    this -> nodes = nodes;
    for (int i = 0; i < nodes; i++) 
      for (int j = 0; j < nodes; j++)
        adjacency[i][j] = false; 
  }

  ~graph()
  {
    for (int i = 0; i < nodes; i++)
      free(adjacency[i]);
    free(adjacency);
  }

  inline void newEdge(int node1,int node2)
  {
    adjacency[node1][node2] = true;
  }

  inline bool hasEdge(int node1,int node2)
  {
    return adjacency[node1][node2];
  }

  int visit(int node,node_color *colors,int *path,int pos)
  {
    colors[node] = GREY;
    path[pos++] = node;
    for (int i = 0; i < nodes; i++) {
      if (!hasEdge(node,i)) continue;
      switch (colors[i]) {
        case GREY:
          path[pos++] = i;
          return pos;
        case WHITE:
          int x = visit(i,colors,path,pos); 
          if (x > 0) return x;}}
    colors[node] = BLACK;
    return 0;
  }

  int *filterCycle(int *path,int size)
  {
    int last = path[size - 1];
    int offset;
    for (offset = 0; path[offset] != last; offset++);
    if (offset > 0)
      for (int i = 0,j = offset; j < size; i++,j++)
        path[i] = path[j]; 
    path = (int *)realloc(path,(size - offset + 1) * sizeof(int));
    path[size - offset] = END_OF_PATH;
    return path;
  }

  int *hasCycle()
  {
    node_color *colors = (node_color *)malloc(nodes * sizeof(node_color)); 
    int *path = (int *)malloc((nodes + 2)* sizeof(int));
    int size = 0;
    for (int i = 0; i < nodes; i++)
      colors[i] = WHITE;
    for (int i = 0; i < nodes; i++)
      if (colors[i] == WHITE) {
        size = visit(i,colors,path,0); 
        if (size > 0) return filterCycle(path,size);     
      }
    return NULL;
  }

};

#endif

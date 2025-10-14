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
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "mtl/Vec.h"


class Stack
{

public:

	int *elem,*demand,sp,sum;

	Stack(int n,int *demand)
	{
		elem = (int *)malloc(n * sizeof(int));
		this->demand = demand;
		sp = sum = 0;
	}

	~Stack()
	{
		free(elem);
	}

	void push(int x)
	{
		elem[sp++] = x;
		sum += demand[x];
	}

	void pop()
	{
		sum -= demand[elem[--sp]];
	}

	int get_sum()
	{
		return(sum);
	}

};


class Resource
{

public:

	vec<int> conflicts;

private:

	void push_subsume(Stack *s)
	{
		int i;
		int x = encode_to_pattern(&s->elem[0],s->sp);
		if (x == 0) return;
		for (i = 0; i < conflicts.size(); i++) {
			if ((conflicts[i] & x) == x) {
				conflicts[i] = x;
				break;
			}
			else if ((conflicts[i] & x) == conflicts[i])
				break;
		}
		if (i == conflicts.size())
			conflicts.push(x);
	}

	void constrain_rec(int n,int *demand,int base,int avail,Stack *s)
	{
		for (int i = 0; i < n; i++) {
			if (demand[i] == 0) continue;
			s->push(base + i);
			if (s->get_sum() <= avail)
				constrain_rec(n - i - 1,demand + i + 1,base + i + 1,avail,s);
			else
				push_subsume(s);
			s->pop();
		}
	}


public:

	void constrain(int n,int *demand,int avail)
	{
		Stack s(n,demand);
		constrain_rec(n,demand,0,avail,&s);
	}


	int encode_to_pattern(int *v,int size)
	{
		int x = 0;
		for (int i = 0; i < size; i++)
			x |= 1 << v[i];
		return x;
	}


	int decode_from_pattern(int x,vec<int> &v)
	{
		v.clear();
		int exp = 0;
		while (x > 0) {
		if (x & 1)
		v.push(exp);
		exp++;
		x = x >> 1;
		}
		return v.size();
	}

};

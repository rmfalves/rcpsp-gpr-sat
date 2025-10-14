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
#ifndef SAT_VAR
#define SAT_VAR

#include <iostream>
#include <vector>
#include <string.h>

#define UNDEF -1

using namespace std;

struct SatVar
{
	int value,cnf_index;
};


class SatMatrix
{
	vector<vector<SatVar> > elem;
	char x_title,y_title;
	char fill_token[5];
	
	public:

	int base_index,rows,columns;

	void Build(int base_index,int rows,int columns,char x_title,char y_title,const char *fill_token)
	{
		this->base_index = base_index;
		this->rows = rows;
		this->columns = columns;
		this->x_title = x_title;
		this->y_title = y_title;
		strcpy(this->fill_token,fill_token);
		elem.resize(rows);
		for (int i = 0; i < rows; i++) {
			elem[i].resize(columns);
			for (int j = 0; j < columns; j++,base_index++) {
				elem[i][j].value = UNDEF;
				elem[i][j].cnf_index = base_index; }}
	}

	void Build(int base_index,int rows,int columns)
	{
		this->base_index = base_index;
		this->rows = rows;
		this->columns = columns;
		this->x_title = ' ';
		this->y_title = ' ';
		this->fill_token[0] = 0;
		elem.resize(rows);
		for (int i = 0; i < rows; i++) {
			elem[i].resize(columns);
			for (int j = 0; j < columns; j++,base_index++) {
				elem[i][j].value = UNDEF;
				elem[i][j].cnf_index = base_index; }}
	}

  SatMatrix() {rows = columns = 0;}

	~SatMatrix() {for (int i = 0; i < rows; i++) elem[i].clear(); elem.clear();}

	int getElem(int row,int column) {return elem[row][column].value;}

	void setElem(int row,int column,int value) {elem[row][column].value = value;}

	int cnfVar(int row,int column) {return elem[row][column].cnf_index;}

	void loadFromList(vector<int> &list)
	{
		for (int i = 0, offset = base_index - 1; i < rows; i++)
			for (int j = 0; j < columns; j++, offset++)
				elem[i][j].value = list[offset];
	}

	void report(FILE *f)
	{
		fprintf(f,"\n     ");
		for (int j = 0; j < columns; j++)
			fprintf(f,"%c%02d ",x_title,j + 1);		
		fprintf(f,"\n");
		for (int j = columns; j >= 0; j--)
			fprintf(f,"====");
		fprintf(f,"\n");
		for (int i = 0; i < rows; i++) {
			fprintf(f,"%c%02d|",y_title,i + 1);		
			for (int j = 0; j < columns; j++)
				fprintf(f,"%4s",elem[i][j].value ? fill_token : "");
			fprintf(f,"\n"); }
		fprintf(f,"\n");
	}
					
};

#endif


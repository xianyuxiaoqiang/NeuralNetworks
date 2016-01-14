#ifndef MY_COMMON_H_DEF
#define MY_COMMON_H_DEF

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<time.h>
#include "png.h"
#include "zlib.h"
#define MAX_COL_WIDTH 10      // the max width of one column
#define DEBUG_MSG 1

#define DATA_INPUT_ROW_NUM 16  // TODO: More rows of input text data
#define DATA_INPUT_COL_NUM 16  // TODO: More columns of input text data
typedef struct input_data_tag
{
	unsigned int row_num;
	unsigned int col_num;
	unsigned int col_width;
	char *data_p;
} input_data_t;

typedef struct input_data_recognition_tag
{
	double data[DATA_INPUT_ROW_NUM * DATA_INPUT_COL_NUM];  // TODO: More rows of input text data;  TODO: More columns of input text data //performance update
} input_data_recognition_t;


void error_output( char *msg );
input_data_t initInputData(char * fname, unsigned int col_width);
input_data_recognition_t initInputDataRecognition(char * fname);
input_data_recognition_t initInputDataRecPNG(char * fname);
void freeInputData(input_data_t *indata_p);

char *getAddress(input_data_t *indata_p, unsigned int col, unsigned int row);

double getActivation( double * xList, unsigned int xCount, double *wList, unsigned int wCount );
double getOutput( const double activation );

int getRand(int mod);

void debug_output( char *msg );

#endif

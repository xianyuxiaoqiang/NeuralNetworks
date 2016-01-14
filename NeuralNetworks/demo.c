#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "population.h"


void repain(input_data_t indata)
{
	unsigned int i;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hOut, &info);
	pos = info.dwSize;
	pos.X = 0;
	pos.Y = 0;
	if( indata.row_num > 0 )
	{
		{
			SetConsoleCursorPosition(hOut, pos);
			for( i=0; i<indata.row_num; i++ )
			{
				printf("%s\n", &indata.data_p[i * ( indata.col_num * 2 + 1 )]);
			}
			Sleep(1000);
		}
	}
}

// 0 : moved
// 1 : hit the wall
int moveUp(input_data_t *indata_p, unsigned int *cur_col_p, unsigned int *cur_row_p)
{
	unsigned int cur_row = *cur_row_p;
	unsigned int cur_col = *cur_col_p;
	char buffer1[MAX_COL_WIDTH];
	char buffer2[MAX_COL_WIDTH];

	if( cur_row == 0 )
	{
		return 1;
	}
	memcpy(buffer1, indata_p->data_p, indata_p->col_width); // the wall
	memcpy(buffer2, getAddress(indata_p, cur_col, cur_row-1), indata_p->col_width);
    if( memcmp( buffer1, buffer2, indata_p->col_width) == 0 )
	{
		return 1;
	}
	// else move to space
	memcpy( getAddress(indata_p, cur_col, cur_row), buffer2, indata_p->col_width );
	memcpy( getAddress(indata_p, cur_col, cur_row-1), buffer1, indata_p->col_width );
	*cur_row_p -= 1;
	return 0;
}
int moveDown(input_data_t *indata_p, unsigned int *cur_col_p, unsigned int *cur_row_p)
{
	unsigned int cur_row = *cur_row_p;
	unsigned int cur_col = *cur_col_p;
	char buffer1[MAX_COL_WIDTH];
	char buffer2[MAX_COL_WIDTH];

	if( cur_row >= indata_p->row_num-1 )
	{
		return 1;
	}
	memcpy(buffer1, indata_p->data_p, indata_p->col_width); // the wall
	memcpy(buffer2, getAddress(indata_p, cur_col, cur_row+1), indata_p->col_width);
    if( memcmp( buffer1, buffer2, indata_p->col_width) == 0 )
	{
		return 1;
	}
	// else move to space
	memcpy( getAddress(indata_p, cur_col, cur_row), buffer2, indata_p->col_width );
	memcpy( getAddress(indata_p, cur_col, cur_row+1), buffer1, indata_p->col_width );
	*cur_row_p += 1;
	return 0;
}
int moveLeft(input_data_t *indata_p, unsigned int *cur_col_p, unsigned int *cur_row_p)
{
	unsigned int cur_row = *cur_row_p;
	unsigned int cur_col = *cur_col_p;
	char buffer1[MAX_COL_WIDTH];
	char buffer2[MAX_COL_WIDTH];

	if( cur_col == 0 )
	{
		return 1;
	}
	memcpy(buffer1, indata_p->data_p, indata_p->col_width); // the wall
	memcpy(buffer2, getAddress(indata_p, cur_col-1, cur_row), indata_p->col_width);
    if( memcmp( buffer1, buffer2, indata_p->col_width) == 0 )
	{
		return 1;
	}
	// else move to space
	memcpy( getAddress(indata_p, cur_col, cur_row), buffer2, indata_p->col_width );
	memcpy( getAddress(indata_p, cur_col-1, cur_row), buffer1, indata_p->col_width );
	*cur_col_p -= 1;
	return 0;
}
int moveRight(input_data_t *indata_p, unsigned int *cur_col_p, unsigned int *cur_row_p)
{
	unsigned int cur_row = *cur_row_p;
	unsigned int cur_col = *cur_col_p;
	char buffer1[MAX_COL_WIDTH];
	char buffer2[MAX_COL_WIDTH];

	if( cur_col >= indata_p->col_num-1 )
	{
		return 1;
	}
	memcpy(buffer1, indata_p->data_p, indata_p->col_width); // the wall
	memcpy(buffer2, getAddress(indata_p, cur_col+1, cur_row), indata_p->col_width);
    if( memcmp( buffer1, buffer2, indata_p->col_width) == 0 )
	{
		return 1;
	}
	// else move to space
	memcpy( getAddress(indata_p, cur_col, cur_row), buffer2, indata_p->col_width );
	memcpy( getAddress(indata_p, cur_col+1, cur_row), buffer1, indata_p->col_width );
	*cur_col_p += 1;
	return 0;
}

char decode_gene(char gene)
{
	return ( gene & 0xC0 );
}

// init the population with random gene
int random_population( population_t *popu_p )
{
	unsigned int i,j;
	chromo_t *chromo_p;
	char *data_p;
	for( i=0; i<popu_p->individual_num; i++ )
	{
		chromo_p = &popu_p->individual_array[i].chromo;
		data_p = chromo_p->data_p;
		for( j=0; j<(chromo_p->bits_per_gene*chromo_p->gene_num+8)/8; j++ )
		{
			data_p[j] = (char)getRand(256);
		}
	}
	return 0;
}
int random_population_neuralnetwork( population_neuralnetwork_t *popu_p, int use_saved )
{
	unsigned int i,j,k;
	neuralnetwork_t *chromo_p;
	int randnum = 1000; // todo : change this number. It is importent!
	double magic = 0.001;
	int ran;
	char namebuf[20];

	for( i=0; i<popu_p->individual_num; i++ )
	{
		chromo_p = &popu_p->individual_array[i];
		for( j=0; j<chromo_p->hide_neural_num; j++ )
		{
			for( k=0; k<chromo_p->hide_neural_p[j].input_num; k++ )
			{
				ran = getRand(randnum);
				if( ran < randnum / 2 )
				{
                    ran *= -1;
				}
				else
				{
					ran -= randnum / 2;
				}
				chromo_p->hide_neural_p[j].power_p[k] = magic * ran; // the magic number
			}
		}
		for( j=0; j<chromo_p->output_neural_num; j++ )
		{
			for( k=0; k<chromo_p->output_neural_p[j].input_num; k++ )
			{
				ran = getRand(randnum);
				if( ran < randnum / 2 )
				{
                    ran *= -1;
				}
				else
				{
					ran -= randnum / 2;
				}
				chromo_p->output_neural_p[j].power_p[k] = magic * ran;
			}// performance update
		}
	}
	if( use_saved )
	{
		for( i=0; i<popu_p->individual_num; i++ )
		{
			sprintf(namebuf, NEURAL_SAVING_FILE_FMT, i);
			loadNeuralnetwork( &popu_p->individual_array[i], namebuf );
		}
	}
	return 0;
}

// active once with all individuals
// 0 : no one success
// 1 : some one success (with score 100)
int active( population_t *popu_p )
{
	unsigned int i,j,cur_col, cur_row;
	int ret;
	float score;
	float max_score = 0;
	int max_index=0;
	chromo_t *chromo_p;
	population_t next_popu;
	// caculate score of all individual
	for( i=0; i<popu_p->individual_num; i++ )
	{
		ret = updateScore(&popu_p->individual_array[i]);
		
		if( ret == 1 )
		{
			debug_output("active: success!");
			return 1;
		}
	}

	// create the next genaration
	if( init_population( &next_popu, popu_p->individual_num ) == 0 )
	{
		crossover(popu_p, &next_popu);
		// after crossover, change popu_p to the next generation
		for( i=0; i<popu_p->individual_num; i++ )
		{
			char *tmp;
			tmp = next_popu.individual_array[i].chromo.data_p;
			next_popu.individual_array[i].chromo.data_p = popu_p->individual_array[i].chromo.data_p;
			popu_p->individual_array[i].chromo.data_p = tmp;
		}
		
		for( i=0; i<popu_p->individual_num; i++ )
		{
			if( popu_p->individual_array[i].score > max_score )
			{
				max_score = popu_p->individual_array[i].score;
				max_index = i;
			}
		}
		free_population(&next_popu);
	}
	if( max_score >= 90 )
	{
		debug_output("near the end!");
	}
	mutation( popu_p );
	return 0;
}
int active_neuralnetwork( population_neuralnetwork_t *popu_p, input_data_recognition_t* data_p, int data_num )
{
	unsigned int i,j,cur_col, cur_row;
	int ret;
	float score;
	float max_score = 0;
	int max_index=0;
	neuralnetwork_t *chromo_p;
	population_neuralnetwork_t next_popu;
	// caculate score of all individual
	for( i=0; i<popu_p->individual_num; i++ )
	{
		ret = updateScore_neuralnetwork(&popu_p->individual_array[i], data_p, data_num);
		
		if( ret == 1 )
		{
			debug_output("active: success!");
			return 1;
		}
	}

	crossover_neuralnetwork( popu_p, data_p, data_num );

	mutation_neuralnetwork( popu_p, data_p, data_num );
	popu_p->min_score = popu_p->individual_array[0].score;
	for( i=0; i<popu_p->individual_num; i++ )
	{
		if( popu_p->individual_array[i].score > max_score )
		{
			max_score = popu_p->individual_array[i].score;
			max_index = i;
		}
		if( popu_p->individual_array[i].score < popu_p->min_score )
		{
			popu_p->min_score = popu_p->individual_array[i].score;
		}
	}
	popu_p->max_score = max_score;
	return 0;
}

int showResult( chromo_t *chromo_p )
{
	unsigned int i,j,cur_col, cur_row;
	input_data_t indata;
	indata = initInputData("input.txt",2);
	cur_col = 0;
	cur_row = 1;
	for( i=0; i<chromo_p->gene_num; i++ )
	{
		char gene;
		char direction;

		gene = get_gene(chromo_p, i);
		direction = decode_gene(gene);
		switch(direction)
		{
		case MOVE_UP:
			moveUp( &indata, &cur_col, &cur_row );
			repain(indata);
			break;
		case MOVE_DOWN:
			moveDown( &indata, &cur_col, &cur_row );
			repain(indata);
			break;
		case MOVE_LEFT:
			moveLeft( &indata, &cur_col, &cur_row );
			repain(indata);
			break;
		case MOVE_RIGHT:
			moveRight( &indata, &cur_col, &cur_row );
			repain(indata);
			break;
		default:
			debug_output("active: error direction.");
			break;
		}
		if( cur_col == indata.col_num - 1 )
		{
			debug_output("active: success!");
			break;
		}
	}
	return 0;
}
int showResulNeural( neuralnetwork_t *chromo_p )
{
	printf( "The number is near : %f\n", chromo_p->output_neural_p[0].output );
	return 0;
}

int main_without_neural( int argc, char ** argv )
{
	population_t popu;
	int i;
	int ret;

	init_population( &popu, 20 );
	random_population( &popu );

    for( i=0; i<MAX_GENERATION; i++ )
	{
		ret = active( &popu );
		if( ret == 1 )
		{
			printf("success at %d genaration.\n", i+1);
			break;
		}
	}
	if( ret == 1 )
	{
		for( i=0; i<popu.individual_num; i++ )
		{
			if( popu.individual_array[i].score >= SUCCESS_SCORE )
			{
				showResult(&popu.individual_array[i].chromo);
				break;
			}
		}
	}

	free_population(&popu);

    return 0;
}
void recognize_number_test(char *inputfile, neuralnetwork_t *chromo_p)
{
	int i;
	double out;
	input_data_recognition_t data;
	data = initInputDataRecognition(inputfile);
	for( i=0; i<sizeof(data.data); i++ )
	{
		chromo_p->input_data_p[i] = data.data[i];
	}
	out = getOutputOfNeuralnetwork(chromo_p);
	printf( "I guess the number is : %d\n", 
		((out - (int)out) > 0.5 ? (int)out + 1 : (int)out)
		);
}
void recognize_number_test_in_png(char *inputfile, neuralnetwork_t *chromo_p)
{
	int i;
	double out;
	input_data_recognition_t data;
	data = initInputDataRecPNG(inputfile);
	memcpy(chromo_p->input_data_p, &data.data, sizeof(data.data) );
	out = getOutputOfNeuralnetwork(chromo_p);
	printf( "I guess the number is : %d\n", 
		((out - (int)out) > 0.5 ? (int)out + 1 : (int)out)
		);
}
// test numbers from 1 to 9 in static text files
int test_static_numbers( int argc, char ** argv )
{
	population_neuralnetwork_t popu;
	int i;
	int ret;
	input_data_recognition_t data[10];
	char filename[100];

	init_population_neuralnetwork( &popu, 100);
	//random_population_neuralnetwork( &popu, FALSE );
	random_population_neuralnetwork( &popu, TRUE );
	for( i=0; i<10; i++ )
	{
		sprintf(filename, "recognize%d.txt",i+1);
		data[i] = initInputDataRecognition(filename);
	}

    for( i=0; i<MAX_GENERATION; i++ )
	{
		ret = active_neuralnetwork( &popu, data, 10 );
		if( ret == 1 )
		{
			printf("success at %d genaration.\n", i+1);
			break;
		}
		Sleep(1);
	}
	if( ret == 1 )
	{
		for( i=0; i<popu.individual_num; i++ )
		{
			if( popu.individual_array[i].score >= SUCCESS_SCORE )
			{
				saveMaxScoreNeuralnetwork( &popu, "save.log" );
				showResulNeural(&popu.individual_array[i]);
				recognize_number_test("recognize1.txt", &popu.individual_array[i]);
				recognize_number_test("recognize2.txt", &popu.individual_array[i]);
				recognize_number_test("recognize3.txt", &popu.individual_array[i]);
				recognize_number_test("recognize4.txt", &popu.individual_array[i]);
				recognize_number_test("recognize5.txt", &popu.individual_array[i]);
				recognize_number_test("recognize6.txt", &popu.individual_array[i]);
				recognize_number_test("recognize7.txt", &popu.individual_array[i]);
				recognize_number_test("recognize8.txt", &popu.individual_array[i]);
				recognize_number_test("recognize9.txt", &popu.individual_array[i]);
				recognize_number_test("recognize10.txt", &popu.individual_array[i]);
				recognize_number_test("Recognition.txt", &popu.individual_array[i]);
				break;
			}
		}
	}

	free_population_neuralnetwork(&popu);

    return 0;
}
int main( int argc, char ** argv )
{
	population_neuralnetwork_t popu;
	int i;
	int ret;
	input_data_recognition_t data[10];
	char filename[100];
	
	init_population_neuralnetwork( &popu, 100);
	random_population_neuralnetwork( &popu, FALSE );
	//random_population_neuralnetwork( &popu, TRUE );
	for( i=0; i<10; i++ )
	{
		sprintf(filename, "%d.png",i+1);
		data[i] = initInputDataRecPNG(filename);
	}

    for( i=0; i<MAX_GENERATION; i++ )
	{
		ret = active_neuralnetwork( &popu, data, 10 );
		if( ret == 1 )
		{
			printf("success at %d genaration.\n", i+1);
			break;
		}
		Sleep(1);
	}
	if( ret == 1 )
	{
		for( i=0; i<popu.individual_num; i++ )
		{
			if( popu.individual_array[i].score >= SUCCESS_SCORE )
			{
				saveMaxScoreNeuralnetwork( &popu, "save.log" );
				showResulNeural(&popu.individual_array[i]);
				recognize_number_test_in_png("1.png", &popu.individual_array[i]);
				recognize_number_test_in_png("2.png", &popu.individual_array[i]);
				recognize_number_test_in_png("3.png", &popu.individual_array[i]);
				recognize_number_test_in_png("4.png", &popu.individual_array[i]);
				recognize_number_test_in_png("5.png", &popu.individual_array[i]);
				recognize_number_test_in_png("6.png", &popu.individual_array[i]);
				recognize_number_test_in_png("7.png", &popu.individual_array[i]);
				recognize_number_test_in_png("8.png", &popu.individual_array[i]);
				recognize_number_test_in_png("9.png", &popu.individual_array[i]);
				recognize_number_test_in_png("10.png", &popu.individual_array[i]);
				recognize_number_test_in_png("Recognition.png", &popu.individual_array[i]);
				break;
			}
		}
	}
	free_population_neuralnetwork(&popu);
    return 0;
}
void test_gene()
{
	chromo_t chromo;
	char gene = 0x7F;
	
	init_chromo(&chromo, 2, 20);
	set_gene(&chromo, 3,gene);
	set_gene(&chromo, 4,gene);
	set_gene(&chromo, 5,gene);
	gene = get_gene(&chromo, 3);
	printf("%0x\n", gene);
	gene = get_gene(&chromo, 4);
	printf("%0x\n", gene);
	gene = get_gene(&chromo, 5);
	printf("%0x\n", gene);
	free_chromo(&chromo);
}

void test_input()
{
	FILE *inFile = NULL;
    double result = 0;
	double output = 0;
	double xList[5];
	double wList[5];
	int i;
	int size = 0;
	input_data_t indata;
	unsigned int row=1;
	unsigned int col=0;
	indata = initInputData("input.txt",2);
	repain(indata);

	for( i = 0; i < 100; i++ )
	{
		int ret = getRand(4);
		switch (ret)
		{
		case 0:
			moveUp(&indata, &col, &row);
			break;
		case 1:
			moveRight(&indata, &col, &row);
			break;
		case 2:
			moveDown(&indata, &col, &row);
			break;
		case 3:
			moveLeft(&indata, &col, &row);
			break;
		default:
			break;
		}
		repain(indata);
	}


	for( i = 0; i < 5; i++ )
	{
		xList[i] = i;
		wList[i] = 0.2;
	}
    result = getActivation( xList, 5, wList, 5 );
	printf( "Activation = %f\n", result );
	output = getOutput( result );
	printf( "Output = %f\n", output );

	freeInputData(&indata);
}


#ifndef MY_POPULATION_H_DEF
#define MY_POPULATION_H_DEF

#include "chromosome.h"
#include "neuralnetwork.h"


#define BITS_PER_GENE 2   // todo1: muti gene. todo2: input from config file
#define GEN_NUM      30   // todo1: muti gene. todo2: input from config file
#define HIDE_NEURAL_NUM  4  // todo: input from config file
#define OUTPUT_NEURAL_NUM 1  // todo: input from config file
#define NEURAL_INPUT_NUM (DATA_INPUT_ROW_NUM * DATA_INPUT_COL_NUM)  // todo: input from config file
#define NEURAL_SAVING_FILE_FMT "save%d.log" // todo: input from config file
#define MAX_POWER     6   // TODO : change the max power to get better performance
#define MIN_POWER     -6   // TODO : change the max power to get better performance
#define SUCCESS_SCORE  97.0
#define MINI_PLUS     0.0001
#define MINI_MINUS    -0.0001
#define GROUP_NUMBER  10  // the number of one group. One group of neuralnetworks only deal with a special input

#define MOVE_UP    (char)0x00
#define MOVE_DOWN  (char)0x40
#define MOVE_LEFT  (char)0x80
#define MOVE_RIGHT (char)0xC0

#define MAX_GENERATION 3000000

#define MutationRate 0.5
#define CrossoverRate  0.7


typedef struct individual_tag
{
	chromo_t chromo;
	float score;   // the score of the individual(0~100)
}individual_t;


typedef struct population_tag
{
	individual_t *individual_array;
	unsigned int individual_num;
}population_t;

typedef struct population_neuralnetwork_tag
{
	neuralnetwork_t *individual_array;
	unsigned int individual_num;
	float max_score;
	float min_score; // for debug
	int group_success_flag[GROUP_NUMBER];
}population_neuralnetwork_t;



typedef struct rouletteWheelPiece_tag
{
	float low;
	float high;
}piece_t;
typedef struct rouletteWheel_tag
{
	unsigned int num;
	piece_t *data_p;
}rouletteWheel_t;


int init_population( population_t *popu_p, unsigned int individual_num);
int init_population_neuralnetwork( population_neuralnetwork_t *popu_p, unsigned int individual_num);
int free_population( population_t *popu_p );
int free_population_neuralnetwork( population_neuralnetwork_t *popu_p );


int crossover( population_t *parents_p, population_t *children_p );
int crossover_neuralnetwork( population_neuralnetwork_t *parents_p, input_data_recognition_t* data_p, int data_num );
int updateScore( individual_t *indi );
int updateScore_neuralnetwork( neuralnetwork_t *indi, input_data_recognition_t* data_p, int data_num );
int tryRemember_neuralnetwork( neuralnetwork_t *indi, input_data_recognition_t* data_p, int data_num );
int mutation( population_t *parents_p );
int mutation_neuralnetwork( population_neuralnetwork_t *parents_p, input_data_recognition_t* data_p, int data_num );
double mutation_neuralnetwork_sub( double a );

int saveMaxScoreNeuralnetwork( population_neuralnetwork_t *pop_p , char *filename);

int loadNeuralnetwork( neuralnetwork_t *chromo_p , char *filename);

#endif

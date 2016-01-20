#include "population.h"
#include "common.h"


population_neuralnetwork_t next_popu;
int next_popu_init_flag = 0;
population_neuralnetwork_t toA,toB;
int toA_toB_init_flag = 0;
rouletteWheel_t rw;
int rw_init_flag = 0;

int init_population( population_t *popu_p, unsigned int individual_num)
{
	unsigned int i;
	int ret;
	popu_p->individual_array = NULL;
	popu_p->individual_num = 0;
	popu_p->individual_array = (individual_t*)malloc( sizeof(individual_t) * individual_num );
	if( popu_p->individual_array == NULL )
	{
		error_output( "init_population: malloc error." );
		goto ERROR_END;
	}
	for( i=0; i<individual_num; i++ )
	{
		// todo1: muti gene. todo2: input from config file
		ret = init_chromo( &popu_p->individual_array[i].chromo, BITS_PER_GENE, GEN_NUM );
		if( ret != 0 )
		{
			goto ERROR_END;
		}
		popu_p->individual_array[i].score = 0;
	}
	popu_p->individual_num = individual_num;
	return 0;
ERROR_END:
	free_population(popu_p);
	return 1;
}
int init_population_neuralnetwork( population_neuralnetwork_t *popu_p, unsigned int individual_num)
{
	unsigned int i,j;
	int ret;
	popu_p->individual_array = NULL;
	popu_p->individual_num = 0;
	popu_p->individual_array = (neuralnetwork_t*)malloc( sizeof(neuralnetwork_t) * individual_num );
	if( popu_p->individual_array == NULL )
	{
		error_output( "init_population_neuralnetwork: malloc error." );
		goto ERROR_END;
	}
	for( i=0; i<individual_num; i++ )
	{
		popu_p->individual_array[i].hide_neural_num = HIDE_NEURAL_NUM;
		popu_p->individual_array[i].input_num = NEURAL_INPUT_NUM+1;// add special input
		popu_p->individual_array[i].output_neural_num = OUTPUT_NEURAL_NUM;
		popu_p->individual_array[i].group_id = i / GROUP_NUMBER + 1;   // add group id

		popu_p->individual_array[i].hide_neural_p = (hide_neural_t*)malloc( sizeof( hide_neural_t ) * HIDE_NEURAL_NUM );
		popu_p->individual_array[i].input_data_p = (double *)malloc( sizeof( double ) * (NEURAL_INPUT_NUM+1) );// performance update// add special input
		popu_p->individual_array[i].input_data_p[NEURAL_INPUT_NUM] = -1; // add special input
		popu_p->individual_array[i].output_neural_p = (output_neural_t*)malloc( sizeof( output_neural_t ) * OUTPUT_NEURAL_NUM );
		if( popu_p->individual_array[i].hide_neural_p == NULL
			|| popu_p->individual_array[i].input_data_p == NULL
			|| popu_p->individual_array[i].output_neural_p == NULL )
		{
			error_output( "init_population_neuralnetwork: malloc error." );
			goto ERROR_END;
		}
		for( j=0; j<HIDE_NEURAL_NUM; j++ )
		{
			popu_p->individual_array[i].hide_neural_p[j].input_num = NEURAL_INPUT_NUM+1;// add special input
			popu_p->individual_array[i].hide_neural_p[j].input_data_p = popu_p->individual_array[i].input_data_p;  // todo : get out the risk of mem violation
			popu_p->individual_array[i].hide_neural_p[j].power_p = (double *)malloc( sizeof( double ) * (NEURAL_INPUT_NUM+1) );// performance update// add special input
		}
		for( j=0; j<OUTPUT_NEURAL_NUM; j++ )
		{
			popu_p->individual_array[i].output_neural_p[j].input_num = HIDE_NEURAL_NUM+1;// performance update
			popu_p->individual_array[i].output_neural_p[j].input_data_p = popu_p->individual_array[i].hide_neural_p;
			popu_p->individual_array[i].output_neural_p[j].power_p = (double *)malloc( sizeof( double ) * (HIDE_NEURAL_NUM+1) );// performance update
		}
		for( j=0; j<HIDE_NEURAL_NUM; j++ )
		{
			if( popu_p->individual_array[i].hide_neural_p[j].power_p == NULL )
			{
				error_output( "init_population_neuralnetwork: malloc error." );
				goto ERROR_END;
			}
		}
		for( j=0; j<OUTPUT_NEURAL_NUM; j++ )
		{
			if( popu_p->individual_array[i].output_neural_p[j].power_p == NULL )
			{
				error_output( "init_population_neuralnetwork: malloc error." );
				goto ERROR_END;
			}
		}
		popu_p->individual_array[i].score = 0;
	}
	popu_p->individual_num = individual_num;
	popu_p->max_score = 0;
	
	random_population_neuralnetwork( popu_p, FALSE );
	return 0;
ERROR_END:
	free_population_neuralnetwork(popu_p);
	return 1;
}
int free_population( population_t *popu_p )
{
	unsigned int i;
	if( popu_p->individual_array != NULL )
	{
		for( i=0; i<popu_p->individual_num; i++ )
		{
			free_chromo(&popu_p->individual_array[i].chromo);
		}
		free( popu_p->individual_array );
		popu_p->individual_array = NULL;
		popu_p->individual_num = 0;
	}
	return 0;
}
int free_population_neuralnetwork( population_neuralnetwork_t *popu_p )
{
	unsigned int i,j;
	if( popu_p->individual_array != NULL )
	{
		for( i=0; i<popu_p->individual_num; i++ )
		{
			if( popu_p->individual_array[i].hide_neural_p != NULL )
			{
				for( j=0; j<popu_p->individual_array[i].hide_neural_num; j++ )
				{
					if( popu_p->individual_array[i].hide_neural_p[j].power_p != NULL )
					{
						free( popu_p->individual_array[i].hide_neural_p[j].power_p );
						popu_p->individual_array[i].hide_neural_p[j].power_p = NULL;
					}
				}
				free( popu_p->individual_array[i].hide_neural_p );
				popu_p->individual_array[i].hide_neural_p = NULL;
				popu_p->individual_array[i].hide_neural_num = 0;
			}
			if( popu_p->individual_array[i].input_data_p != NULL )
			{
				free( popu_p->individual_array[i].input_data_p );
				popu_p->individual_array[i].input_data_p = NULL;
				popu_p->individual_array[i].input_num = 0;
			}
			if( popu_p->individual_array[i].output_neural_p != NULL )
			{
				for( j=0; j<popu_p->individual_array[i].output_neural_num; j++ )
				{
					if( popu_p->individual_array[i].output_neural_p[j].power_p != NULL )
					{
						free( popu_p->individual_array[i].output_neural_p[j].power_p );
						popu_p->individual_array[i].output_neural_p[j].power_p = NULL;
					}
				}
				free( popu_p->individual_array[i].output_neural_p );
				popu_p->individual_array[i].output_neural_p = NULL;
				popu_p->individual_array[i].output_neural_num = 0;
			}
		}
		free( popu_p->individual_array );
		popu_p->individual_array = NULL;
		popu_p->individual_num = 0;
	}
	return 0;
}

int rouletteWheelSelection(rouletteWheel_t *rw_p)
{
	int magic = 10000;
	int ran = getRand(magic);
	float cur = rw_p->data_p[rw_p->num -1].high / magic * ran;
	int i;
	for( i=0; i<rw_p->num; i++ )
	{
		if( rw_p->data_p[i].high >= cur )
		{
			return i;
		}
	}
	return 0;
}
int crossover_sub(chromo_t *a, chromo_t *b)
{
	unsigned int bytenum = ( a->bits_per_gene * a->gene_num + 8 ) / 8;
	int ran = getRand(bytenum);
	int i;
	char tmp;
	for( i=ran; i<bytenum; i++ )
	{
		tmp = b->data_p[i];
		b->data_p[i] = a->data_p[i];
		a->data_p[i] = tmp;
	}
	return 0;
}
int crossover_sub_neuralnetwork(neuralnetwork_t *a, neuralnetwork_t *b)
{
	unsigned int genenum = a->hide_neural_num * a->input_num + a->hide_neural_num * a->output_neural_num + 1; // todo : special input for more output
	int ran = getRand(genenum);
	int i;
	double tmp;
	int neural_index, input_index;
	for( i=ran; i<genenum; i++ )
	{
		if( a->hide_neural_num * a->input_num > i )
		{
			// hide neurals
			neural_index = i / a->input_num;
			input_index = i - neural_index * a->input_num;
#ifdef DEBUG_MSG
			if( b->hide_neural_p[neural_index].power_p[input_index] < 0 && a->hide_neural_p[neural_index].power_p[input_index] > 0 
				|| b->hide_neural_p[neural_index].power_p[input_index] >0 && a->hide_neural_p[neural_index].power_p[input_index] < 0 )
			{
				printf(" crossover_sub_neuralnetwork : internal error! \n");
			}
#endif
			tmp = b->hide_neural_p[neural_index].power_p[input_index];
			b->hide_neural_p[neural_index].power_p[input_index] = a->hide_neural_p[neural_index].power_p[input_index];
			a->hide_neural_p[neural_index].power_p[input_index] = tmp;
		}
		else if( a->hide_neural_num * a->input_num + a->hide_neural_num * a->output_neural_num > i )
		{
			neural_index = (i - a->hide_neural_num * a->input_num) / a->hide_neural_num;
			input_index = i - a->hide_neural_num * a->input_num - neural_index * a->hide_neural_num;
#ifdef DEBUG_MSG
			if( b->hide_neural_p[neural_index].power_p[input_index] < 0 && a->hide_neural_p[neural_index].power_p[input_index] > 0 
				|| b->hide_neural_p[neural_index].power_p[input_index] >0 && a->hide_neural_p[neural_index].power_p[input_index] < 0 )
			{
				printf(" crossover_sub_neuralnetwork : internal error! \n");
			}
#endif
			tmp = b->output_neural_p[neural_index].power_p[input_index];
			b->output_neural_p[neural_index].power_p[input_index] = a->output_neural_p[neural_index].power_p[input_index];
			a->output_neural_p[neural_index].power_p[input_index] = tmp;
		}
		else
		{
			// special input
			tmp = b->output_neural_p[0].power_p[b->output_neural_p[0].input_num-1];
			b->output_neural_p[0].power_p[b->output_neural_p[0].input_num-1] =
				a->output_neural_p[0].power_p[a->output_neural_p[0].input_num-1];
			a->output_neural_p[0].power_p[a->output_neural_p[0].input_num-1] = tmp;
		}
	}
	return 0;
}
int crossover( population_t *parents_p, population_t *children_p )
{
	int j;
	int index1;
	int index2;
	int child_index=0;
	int ran;
	rouletteWheel_t rw;

	// init rouletteWheel
	rw.num = parents_p->individual_num;
	rw.data_p = malloc( sizeof(piece_t) * rw.num );
	rw.data_p[0].low = 0;
	rw.data_p[0].high = parents_p->individual_array[0].score;
	if( rw.data_p == NULL )
	{
		error_output( "crossover: malloc error." );
		return 1;
	}
	for( j=1; j<rw.num; j++ )
	{
		rw.data_p[j].low = rw.data_p[j-1].high;
		rw.data_p[j].high = rw.data_p[j].low + parents_p->individual_array[j].score;
	}

	while( child_index < children_p->individual_num-1 )
	{
		index1 = rouletteWheelSelection(&rw);
		index2 = rouletteWheelSelection(&rw);
		ran = getRand(100);
		if( ran > CrossoverRate * 100 )
		{
			continue;
		}

		if( index1 == index2 )
		{
			// this guy is luky
			chromo_t * fromchromo = &parents_p->individual_array[index1].chromo;
			chromo_t * tochromo = &children_p->individual_array[child_index].chromo;
			memcpy( tochromo->data_p, fromchromo->data_p,(tochromo->bits_per_gene * tochromo->gene_num + 8)/8 );
			child_index++;
		}
		else
		{
			// crossover
			chromo_t * fromA = &parents_p->individual_array[index1].chromo;
			chromo_t * fromB = &parents_p->individual_array[index2].chromo;
			chromo_t toA,toB;
			chromo_t * tochromo = &children_p->individual_array[child_index].chromo;
			individual_t indA;
			individual_t indB;
			init_chromo(&toA, fromA->bits_per_gene, fromA->gene_num);
			init_chromo(&toB, fromB->bits_per_gene, fromB->gene_num);
			indA.chromo = toA;
			indB.chromo = toB;
			memcpy( toA.data_p, fromA->data_p,(toA.bits_per_gene * toA.gene_num + 8)/8 );
			memcpy( toB.data_p, fromB->data_p,(toB.bits_per_gene * toB.gene_num + 8)/8 );
			crossover_sub(&toA, &toB);
			// only one the the two can live
			updateScore(&indA);
			updateScore(&indB);
			if( indA.score >= indB.score )
			{
				memcpy( tochromo->data_p, toA.data_p,(tochromo->bits_per_gene * tochromo->gene_num + 8)/8 );
			}
			else
			{
				memcpy( tochromo->data_p, toB.data_p,(tochromo->bits_per_gene * tochromo->gene_num + 8)/8 );
			}
			
			child_index++;

			free_chromo(&toA);
			free_chromo(&toB);
		}
		
		if( child_index >= children_p->individual_num-1 )
		{
			// finish
			break;
		}
	}

	if( rw.data_p != NULL )
	{
		free( rw.data_p );
		rw.data_p = NULL;
	}
	return 0;
}
int crossover_neuralnetwork( population_neuralnetwork_t *parents_p, input_data_recognition_t* data_p, int data_num )
{
	int i,j;
	int index1;
	int index2;
	int child_index=0;
	int ran;
	int expect = 4; // TODO : input from file
	int cur_group; // fix bug : cross by group
	double max_score = parents_p->max_score * 0.6; // the magic number
	population_neuralnetwork_t *children_p = &next_popu;
	
	if( !next_popu_init_flag )
	{
		init_population_neuralnetwork( &next_popu, parents_p->individual_num );
		next_popu_init_flag = 1;
	}
	// init rouletteWheel
	if( !rw_init_flag )
	{
		// TODO : deal with changable individual_num
		rw.num = parents_p->individual_num;
		rw.data_p = (piece_t*)malloc( sizeof(piece_t) * rw.num );
		rw_init_flag = 1;
	}
	if( rw.data_p == NULL )
	{
		error_output( "crossover: malloc error." );
		return 1;
	}
	if( !toA_toB_init_flag )
	{
		init_population_neuralnetwork(&toA, 1);
		init_population_neuralnetwork(&toB, 1);
		toA_toB_init_flag = 1;
	}
	while( child_index < children_p->individual_num )
	{
		{ // fix bug : cross by group
			cur_group = child_index / GROUP_NUMBER + 1;
			rw.data_p[0].low = 0;
			rw.data_p[0].high = parents_p->individual_array[(cur_group-1) * GROUP_NUMBER].score;
			for( j=1; j<GROUP_NUMBER; j++ )
			{
				rw.data_p[j].low = rw.data_p[j-1].high;
				rw.data_p[j].high = rw.data_p[j].low + parents_p->individual_array[(cur_group-1) * GROUP_NUMBER+j].score;
			}
		}
		index1 = rouletteWheelSelection(&rw);
		index2 = rouletteWheelSelection(&rw);

		if( index1 == index2 )
		{
			// this guy is luky
			neuralnetwork_t * fromchromo = &parents_p->individual_array[(cur_group-1) * GROUP_NUMBER + index1];
			neuralnetwork_t * tochromo = &children_p->individual_array[child_index];
			if( fromchromo->score >= max_score )
			{
				// only get upgraded generation
				// todo : fix the risk of mem violation
				for( i=0; i<tochromo->hide_neural_num; i++ )
				{
					memcpy( tochromo->hide_neural_p[i].power_p, fromchromo->hide_neural_p[i].power_p, sizeof(double) * tochromo->hide_neural_p[i].input_num );
				}
				for( i=0; i<tochromo->output_neural_num; i++ )
				{
					memcpy( tochromo->output_neural_p[i].power_p, fromchromo->output_neural_p[i].power_p, sizeof(double) * tochromo->output_neural_p[i].input_num );
				}
				// Fix bug: after cross over, the score turned to minus-max
				tochromo->group_id = fromchromo->group_id;
				tochromo->score = fromchromo->score;
				child_index++;
			}
		}
		else
		{
			// crossover
			neuralnetwork_t * tochromo = NULL;
			neuralnetwork_t * fromA = &parents_p->individual_array[(cur_group-1) * GROUP_NUMBER + index1];
			neuralnetwork_t * fromB = &parents_p->individual_array[(cur_group-1) * GROUP_NUMBER + index2];
			if( fromA->group_id != fromB->group_id )
			{
				// only deal with the same group
				printf("internal error ! fromA->group_id : %d, fromB->group_id : %d\n ", fromA->group_id, fromB->group_id);
				continue;
			}
			// todo : fix the risk of mem violation
			for( i=0; i<toA.individual_array[0].hide_neural_num; i++ )
			{
				memcpy( toA.individual_array[0].hide_neural_p[i].power_p, fromA->hide_neural_p[i].power_p, sizeof(double) * fromA->hide_neural_p[i].input_num );
			}
			for( i=0; i<toA.individual_array[0].output_neural_num; i++ )
			{
				memcpy( toA.individual_array[0].output_neural_p[i].power_p, fromA->output_neural_p[i].power_p, sizeof(double) * fromA->output_neural_p[i].input_num );
			}
			for( i=0; i<toB.individual_array[0].hide_neural_num; i++ )
			{
				memcpy( toB.individual_array[0].hide_neural_p[i].power_p, fromB->hide_neural_p[i].power_p, sizeof(double) * fromB->hide_neural_p[i].input_num );
			}
			for( i=0; i<toB.individual_array[0].output_neural_num; i++ )
			{
				memcpy( toB.individual_array[0].output_neural_p[i].power_p, fromB->output_neural_p[i].power_p, sizeof(double) * fromB->output_neural_p[i].input_num );
			}
			
			// Fix bug : after crossover, the group id is changed because the group id of 'next_popu' is not as the same as parents
			toA.individual_array[0].group_id = fromA->group_id;
			toB.individual_array[0].group_id = fromB->group_id;

			crossover_sub_neuralnetwork(&toA.individual_array[0], &toB.individual_array[0]);
			// only one of the two can live
			// debug end
			updateScore_neuralnetwork(&toA.individual_array[0], data_p, data_num);
			updateScore_neuralnetwork(&toB.individual_array[0], data_p, data_num);
			if( toA.individual_array[0].score >= toB.individual_array[0].score && toA.individual_array[0].score >= max_score) 
			{
				tochromo = &children_p->individual_array[child_index];
				// only good crossover can be the next generation
				for( i=0; i<toA.individual_array[0].hide_neural_num; i++ )
				{
					memcpy( tochromo->hide_neural_p[i].power_p, toA.individual_array[0].hide_neural_p[i].power_p, sizeof(double) * toA.individual_array[0].hide_neural_p[i].input_num );
				}
				for( i=0; i<toA.individual_array[0].output_neural_num; i++ )
				{
					memcpy( tochromo->output_neural_p[i].power_p, toA.individual_array[0].output_neural_p[i].power_p, sizeof(double) *  toA.individual_array[0].output_neural_p[i].input_num );
				}
				tochromo->score = toA.individual_array[0].score;
				// Fix bug : after crossover, the group id is changed because the group id of 'next_popu' is not as the same as parents
				tochromo->group_id = toA.individual_array[0].group_id;
				child_index++;
				if( child_index >= children_p->individual_num )
				{
					// finish
					break;
				}
			}
			if( toB.individual_array[0].score >= toA.individual_array[0].score && toB.individual_array[0].score >= max_score) 
			{
				tochromo = &children_p->individual_array[child_index];
				// only good crossover can be the next generation
				for( i=0; i<toB.individual_array[0].hide_neural_num; i++ )
				{
					memcpy( tochromo->hide_neural_p[i].power_p, toB.individual_array[0].hide_neural_p[i].power_p, sizeof(double) * toB.individual_array[0].hide_neural_p[i].input_num );
				}
				for( i=0; i<toB.individual_array[0].output_neural_num; i++ )
				{
					memcpy( tochromo->output_neural_p[i].power_p, toB.individual_array[0].output_neural_p[i].power_p, sizeof(double) *  toB.individual_array[0].output_neural_p[i].input_num );
				}
				tochromo->score = toB.individual_array[0].score;
				// Fix bug : after crossover, the group id is changed because the group id of 'next_popu' is not as the same as parents
				tochromo->group_id = toB.individual_array[0].group_id;
				child_index++;
				if( child_index >= children_p->individual_num )
				{
					// finish
					break;
				}
			}
		}
	}

	// after crossover, change popu_p to the next generation
	for( i=0; i<parents_p->individual_num; i++ )
	{
		double *tmp;
		// TODO : deal with memery violation
		for( j=0; j<parents_p->individual_array[0].hide_neural_num; j++ )
		{
			tmp = next_popu.individual_array[i].hide_neural_p[j].power_p;
			next_popu.individual_array[i].hide_neural_p[j].power_p = parents_p->individual_array[i].hide_neural_p[j].power_p;
			parents_p->individual_array[i].hide_neural_p[j].power_p = tmp;
		}
		for( j=0; j<parents_p->individual_array[0].output_neural_num; j++ )
		{
			tmp = next_popu.individual_array[i].output_neural_p[j].power_p;
			next_popu.individual_array[i].output_neural_p[j].power_p = parents_p->individual_array[i].output_neural_p[j].power_p;
			parents_p->individual_array[i].output_neural_p[j].power_p = tmp;
		}
		parents_p->individual_array[i].score = next_popu.individual_array[i].score;
		// Fix bug : after crossover, the group id is changed because the group id of 'next_popu' is not as the same as parents
		parents_p->individual_array[i].group_id = next_popu.individual_array[i].group_id;
	}
	return 0;
}

// 0 : not success
// 1 : success (with score 100)
int updateScore( individual_t *indi )
{
	unsigned int i,j,cur_col, cur_row;
	float score1;
	float score2;
	input_data_t indata;
	chromo_t *chromo_p = &indi->chromo;
	indata = initInputData("input.txt",2);
	cur_col = 0;
	cur_row = 1;
	for( j=0; j<chromo_p->gene_num; j++ )
	{
		char gene;
		char direction;

		gene = get_gene(chromo_p, j);
		direction = decode_gene(gene);
		switch(direction)
		{
		case MOVE_UP:
			moveUp( &indata, &cur_col, &cur_row );
			break;
		case MOVE_DOWN:
			moveDown( &indata, &cur_col, &cur_row );
			break;
		case MOVE_LEFT:
			moveLeft( &indata, &cur_col, &cur_row );
			break;
		case MOVE_RIGHT:
			moveRight( &indata, &cur_col, &cur_row );
			break;
		default:
			debug_output("updateScore: error direction.");
			break;
		}
		if( cur_col == indata.col_num - 1 )
		{
			debug_output("updateScore: success!");
			indi->score = 100.0; // the sucess score
			freeInputData(&indata);
			return 1;
		}
	}
	// set the score
	score1 = ( indata.col_num -1 - cur_col ) * 1.0 / indata.col_num * 100.0;
	score1 = 100.0 - score1;
	score2 = ( indata.row_num -1 - cur_row ) * 1.0 / (indata.row_num - 1) * 100.0;
	score2 = 100.0 - score2;
	indi->score = score1 * score2 / 100.0;

	freeInputData(&indata);
	return 0;
}
// todo : muti expect data. Current is only one int
int updateScore_neuralnetwork( neuralnetwork_t *indi, input_data_recognition_t* data_p, int data_num )
{
	unsigned int i,j; // ,cur_col, cur_row;
	float score1;
	float score2 = 1.0;
	int expect;

	neuralnetwork_t *chromo_p = indi;
	indi->score = 0;

	// for each input
	for( j=0; j<data_num; j++ )
	{
		// init input
		//expect = j+1;
		expect = data_p[j].expcet;
		if( expect != chromo_p->group_id )
		{
			// only deal with input specified by group id
			continue;
		}
		memcpy(chromo_p->input_data_p, &data_p[j].data, NEURAL_INPUT_NUM * sizeof(double));
		//cur_col = 0;
		//cur_row = 1;
		for( i=0; i<chromo_p->output_neural_num; i++ )
		{
			// caculate output
			chromo_p->output_neural_p[i].output = getOutputOfNeuralnetwork(chromo_p);
			if( chromo_p->output_neural_p[i].output >= expect )
			{
				score1 = expect / chromo_p->output_neural_p[i].output;
			}
			else
			{
				score1 = chromo_p->output_neural_p[i].output / expect;
			}
			if( score1 < score2 )
			{
				score2 = score1;
			}
		}
	}
	// set the score(muti input)
	indi->score = score2 * 100;
	if( indi->score >= SUCCESS_SCORE )
	{
		debug_output("updateScore_neuralnetwork: success!");
		return 1;
	}

	return 0;
}

int mutation( population_t *parents_p )
{
	int i;
	int gene_index;
	int bit_index;
	char gene;
	char tmp;
	chromo_t *chromo_p;
	for( i=0; i<parents_p->individual_num; i++ )
	{
		int ran = getRand(10000);
		if( ran >= 10000 * MutationRate )
		{
			continue;
		}
		chromo_p = &parents_p->individual_array[i].chromo;
		gene_index = getRand(chromo_p->gene_num);
		gene = get_gene(chromo_p, gene_index);
		bit_index = getRand(chromo_p->bits_per_gene);
		if( bit_index > 0 )
		{
			tmp = ( 0x7F >> (bit_index-1) ) & ( 0xFF << (8-bit_index-1) );
		}
		else
		{
			tmp = 0xC0;
		}
		if( gene & tmp )
		{
			gene &= ~tmp;
		}
		else
		{
			gene |= tmp;
		}
		set_gene(chromo_p, gene_index, gene);
	}
}
int mutation_neuralnetwork( population_neuralnetwork_t *parents_p, input_data_recognition_t* data_p, int data_num )
{
	int i,j,k;
	neuralnetwork_t *chromo_p;
	unsigned int genenum = parents_p->individual_array[0].hide_neural_num * parents_p->individual_array[0].input_num
		+ parents_p->individual_array[0].hide_neural_num * parents_p->individual_array[0].output_neural_num + 1; // todo1 : more than one output
	double tmp;
	int neural_index, input_index;

	if( !next_popu_init_flag )
	{
		init_population_neuralnetwork( &next_popu, parents_p->individual_num );
		next_popu_init_flag = 1;
	}
	// make a clone of generation
	for( i=0; i<parents_p->individual_num; i++ )
	{
		for( j=0; j<parents_p->individual_array[i].hide_neural_num; j++ )
		{
			memcpy(next_popu.individual_array[i].hide_neural_p[j].power_p, parents_p->individual_array[i].hide_neural_p[j].power_p,
				parents_p->individual_array[i].input_num * sizeof(double) );
		}
		for( j=0; j<parents_p->individual_array[i].output_neural_num; j++ )
		{
			memcpy(next_popu.individual_array[i].output_neural_p[j].power_p, parents_p->individual_array[i].output_neural_p[j].power_p,
				next_popu.individual_array[i].output_neural_p[j].input_num * sizeof(double) );
		}
		next_popu.individual_array[i].score = parents_p->individual_array[i].score;
	}
	for( i=0; i<next_popu.individual_num; i++ )
	{
		int ran = getRand(10000);
		if( ran >= 10000 * MutationRate )
		{
			continue;
		}
		chromo_p = &next_popu.individual_array[i];
		ran = getRand(genenum);
		if( chromo_p->hide_neural_num * chromo_p->input_num > ran )
		{
			// hide neurals
			neural_index = ran / chromo_p->input_num;
			input_index = ran - neural_index * chromo_p->input_num;
			tmp = chromo_p->hide_neural_p[neural_index].power_p[input_index];
			
			tmp = mutation_neuralnetwork_sub(tmp);
#ifdef DEBUG_MSG
			if( tmp < 0 && chromo_p->hide_neural_p[neural_index].power_p[input_index] > 0 
				|| tmp >0 && chromo_p->hide_neural_p[neural_index].power_p[input_index] < 0 )
			{
				printf(" mutation_neuralnetwork : internal error! \n");
			}
#endif
			chromo_p->hide_neural_p[neural_index].power_p[input_index] = tmp;
		}
		else if( chromo_p->hide_neural_num * chromo_p->input_num + chromo_p->hide_neural_num * chromo_p->output_neural_num > ran )
		{
			neural_index = (ran - chromo_p->hide_neural_num * chromo_p->input_num) / chromo_p->hide_neural_num;
			input_index = ran - chromo_p->hide_neural_num * chromo_p->input_num - neural_index * chromo_p->hide_neural_num;
			tmp = chromo_p->output_neural_p[neural_index].power_p[input_index];
			tmp = mutation_neuralnetwork_sub(tmp);
#ifdef DEBUG_MSG
			if( tmp < 0 && chromo_p->output_neural_p[neural_index].power_p[input_index] > 0 
				|| tmp >0 && chromo_p->output_neural_p[neural_index].power_p[input_index] < 0 )
			{
				printf(" mutation_neuralnetwork : internal error! \n");
			}
#endif
			chromo_p->output_neural_p[neural_index].power_p[input_index] = tmp;
		}
		else
		{
			// the special input
			// todo1 : more than one output
			tmp = chromo_p->output_neural_p[0].power_p[chromo_p->output_neural_p[0].input_num-1];// performance update
			tmp = mutation_neuralnetwork_sub(tmp);
			chromo_p->output_neural_p[0].power_p[chromo_p->output_neural_p[0].input_num-1] = tmp;// performance update
		}
		updateScore_neuralnetwork(&next_popu.individual_array[i], data_p, data_num);
	}
	// use the next generation
	for( i=0; i<parents_p->individual_num; i++ )
	{
		if( parents_p->individual_array[i].score * 0.9 < next_popu.individual_array[i].score ) // magic number to make individuals more different
		{
			for( j=0; j<parents_p->individual_array[i].hide_neural_num; j++ )
			{
				memcpy(parents_p->individual_array[i].hide_neural_p[j].power_p, next_popu.individual_array[i].hide_neural_p[j].power_p,
					parents_p->individual_array[i].input_num * sizeof(double) );
			}
			for( j=0; j<parents_p->individual_array[i].output_neural_num; j++ )
			{
				memcpy(parents_p->individual_array[i].output_neural_p[j].power_p, next_popu.individual_array[i].output_neural_p[j].power_p,
					next_popu.individual_array[i].output_neural_p[j].input_num * sizeof(double) );
			}
			parents_p->individual_array[i].score = next_popu.individual_array[i].score;
		}
	}
}
// to gat mutation of a
// todo : change the algorithm . This is very important!
double mutation_neuralnetwork_sub( double a )
{
	int randnum = 1000; // todo : change this number. It is importent!
	float magic = getRand(200);
	double ret;
	int ran2;
	if( a < 0 )
	{
		// reject
		return a;
	}
	//ran1 = getRand(randnum);// no minus power
	ran2 = getRand(randnum);
	if( ran2 < randnum / 2 )
	{
        ran2 = -1;
	}
	else
	{
		ran2 = 1;
	}
	// magic = ran1 * (randnum + magic*ran2) / randnum;// no minus power
	magic = (randnum + magic*ran2) / randnum;
	ret = a * magic;
	if( ret > MAX_POWER || ret < MIN_POWER)
	{
		// to avoid convergence
		ret = ret / 1.2;   // it should match with magic
	}
	return ret;
}

int saveMaxScoreNeuralnetwork( population_neuralnetwork_t *pop_p , char *TODO) // TODO: get file path from config file 
{
    FILE *fp = NULL;
	int i,j,k;
	double max_score = 0;
	int max_index = 0;
	neuralnetwork_t *chromo_p = NULL;
	char namebuf[20];
	for( k=0; k<pop_p->individual_num; k++ )
	{
		sprintf(namebuf, NEURAL_SAVING_FILE_FMT, k);
		chromo_p = &pop_p->individual_array[k];
		fp = fopen(namebuf, "w");
		if( fp != NULL )
		{
			for( i=0; i<chromo_p->hide_neural_num; i++ )
			{
				for( j=0; j<chromo_p->hide_neural_p[i].input_num; j++ )
				{
					fprintf(fp, "%.10Le\n", chromo_p->hide_neural_p[i].power_p[j]);
				}
			}
			for( i=0; i<chromo_p->output_neural_num; i++ )
			{
				for( j=0; j<chromo_p->hide_neural_num; j++ )
				{
					fprintf(fp, "%.10Le\n", chromo_p->output_neural_p[i].power_p[j]);
				}
				fprintf(fp, "%.10Le\n", chromo_p->output_neural_p[i].power_p[chromo_p->output_neural_p[i].input_num-1] );// performance update
			}
			fclose(fp);
		}
	}
	return 0;
}
int loadNeuralnetwork( neuralnetwork_t *chromo_p , char *filename)
{
    FILE *fp = fopen(filename, "r");
	int i,j;
	if( fp != NULL )
	{
		for( i=0; i<chromo_p->hide_neural_num; i++ )
		{
			for( j=0; j<chromo_p->hide_neural_p[i].input_num; j++ )
			{
				fscanf(fp, "%lf\n", &chromo_p->hide_neural_p[i].power_p[j]);
			}
		}
		for( i=0; i<chromo_p->output_neural_num; i++ )
		{
			for( j=0; j<chromo_p->hide_neural_num; j++ )
			{
				fscanf(fp, "%lf\n", &chromo_p->output_neural_p[i].power_p[j]);
			}
			fscanf(fp, "%lf\n", &chromo_p->output_neural_p[i].power_p[chromo_p->output_neural_p[i].input_num-1] );// performance update
		}
		fclose(fp);
	}
	return 0;
}
// try to remember the expected values
int tryRemember_neuralnetwork( neuralnetwork_t *indi, input_data_recognition_t* data_p, int data_num )
{
	unsigned int i,j;
	float score = 0;
	float score2 = 1.0;
	int expect;
	int reject_flag = 1;
	int ret;
	int target = 0;
	int m,n;
	int update_flag = 0;
	double tmp;

	neuralnetwork_t *chromo_p = indi;
	
	while(reject_flag)
	{
		// init score
		score2 = 1.0;
		reject_flag = 0;
		// 1. deal with all target input
		for( j=0; j<data_num; j++ )
		{
			if( data_p[j].expcet == chromo_p->group_id )
			{
				target = j;
			}
			else
			{
				continue;
			}
			// init flag
			score = 0;
			expect = data_p[target].expcet;
			memcpy(chromo_p->input_data_p, &data_p[target].data, NEURAL_INPUT_NUM * sizeof(double));
			while( score < SUCCESS_SCORE / 100 )
			{
				// caculate output
				for( i=0; i<chromo_p->output_neural_num; i++ )
				{
					chromo_p->output_neural_p[i].output = getOutputOfNeuralnetwork(chromo_p);
					if( chromo_p->output_neural_p[i].output >= expect )
					{
						score = expect / chromo_p->output_neural_p[i].output;
					}
					else
					{
						score = chromo_p->output_neural_p[i].output / expect;
					}
					if( score >= SUCCESS_SCORE / 100 )break;
				}

				// update a power
				update_flag = 0;
				for( m=0; m<chromo_p->hide_neural_num; m++ )
				{
					for( n=0; n<chromo_p->hide_neural_p[m].input_num; n++ )
					{
						if( chromo_p->hide_neural_p[m].input_data_p[n] == 0 )
						{
							continue;
						}
						if( chromo_p->hide_neural_p[m].power_p[n] <= MINI_PLUS && chromo_p->hide_neural_p[m].power_p[n] >= MINI_MINUS )
						{
							chromo_p->hide_neural_p[m].power_p[n] *= getRand(100) + 1;
							update_flag = 1;
						}
						if( update_flag )
						{
							break;
						}
					}
					if( update_flag )
					{
						break;
					}
				}
				if( !update_flag )
				{
					for( m=0; m<chromo_p->output_neural_num; m++ )
					{
						for( n=0; n<chromo_p->output_neural_p[m].input_num; n++ )
						{
							if( chromo_p->output_neural_p[m].power_p[n] <= MINI_PLUS && chromo_p->output_neural_p[m].power_p[n] >= MINI_MINUS )
							{
								chromo_p->output_neural_p[m].power_p[n] *= getRand(100) + 1;
								update_flag = 1;
							}
							if( update_flag )
							{
								break;
							}
						}
						if( update_flag )
						{
							break;
						}
					}
				}
				if( !update_flag )
				{
					float magic = 1.1;
					m = getRand(chromo_p->hide_neural_num + chromo_p->output_neural_num);
					if( m >= chromo_p->hide_neural_num )
					{
						n = getRand(chromo_p->output_neural_p[m-chromo_p->hide_neural_num].input_num);
						chromo_p->output_neural_p[m-chromo_p->hide_neural_num].power_p[n] =
							mutation_neuralnetwork_sub( chromo_p->output_neural_p[m-chromo_p->hide_neural_num].power_p[n] );
					}
					else
					{
						n = getRand(chromo_p->hide_neural_p[m].input_num);
						if( chromo_p->hide_neural_p[m].input_data_p[n] != 0 )
						{
							if( chromo_p->hide_neural_p[m].power_p[n] < 0 )
							{
								// internal error!
								chromo_p->hide_neural_p[m].power_p[n] = MINUS_MAX;
								printf("tryRemember_neuralnetwork : internal error! \n");
							}
							else
							{
								chromo_p->hide_neural_p[m].power_p[n] = mutation_neuralnetwork_sub( chromo_p->hide_neural_p[m].power_p[n] * magic );
							}
						}
					}
				}
			}
			if( score < score2 )
			{
				score2 = score;
			}
		}

		// 2. for each input other than target, check rejecttion
		for( j=0; j<data_num; j++ )
		{
			if( data_p[j].expcet == chromo_p->group_id )
			{
				continue;
			}
			memcpy(chromo_p->input_data_p, &data_p[j].data, NEURAL_INPUT_NUM * sizeof(double));
			// input other than group id should reject
			ret = tryReject_neuralnetwork(chromo_p);
			if( ret )
			{
				// need more try
				reject_flag = 1;
			}
		}
	}
	indi->score = score2 * 100;
	return 0;
}
// the sub function of tryRemember_neuralnetwork()
// to reject the input other than group id
// return 1 : updated(need more try)    0 : not updated(successfully reject)
int tryReject_neuralnetwork( neuralnetwork_t *indi )
{
	unsigned int i,m,n;
	neuralnetwork_t *chromo_p = indi;
	int need_update_f = 0;
	float score;
	float min = -0.5; // todo : change the number to get better performance
	float max = 0.5;  // todo : change the number to get better performance
	float devide = 1.2;    // todo : change the number to get better performance
	
	// caculate output
	for( i=0; i<chromo_p->output_neural_num; i++ )
	{
		chromo_p->output_neural_p[i].output = getOutputOfNeuralnetwork(chromo_p);
		if( chromo_p->output_neural_p[i].output / chromo_p->group_id >= 0.9 
			&& chromo_p->output_neural_p[i].output / chromo_p->group_id <= 1.1 )
		{
			// need to update the power to get output not near group_id
			need_update_f = 1;
		}
	}
	if( !need_update_f )
	{
		return 0;
	}
	// update all the powers of input neurals
	for( m=0; m<chromo_p->hide_neural_num; m++ )
	{
		for( n=0; n<chromo_p->hide_neural_p[m].input_num; n++ )
		{
			if( chromo_p->hide_neural_p[m].input_data_p[n] == 0 )
			{
				continue;
			}
			if( chromo_p->hide_neural_p[m].power_p[n] > max || chromo_p->hide_neural_p[m].power_p[n] < min )
			{
				chromo_p->hide_neural_p[m].power_p[n] = max; 
			}
			else
			{
				chromo_p->hide_neural_p[m].power_p[n] /= devide;
			}
		}
	}
	// deal with special power of output
	/*for( m=0; m<chromo_p->output_neural_num; m++ )
	{
		if( chromo_p->output_neural_p[m].power_p[chromo_p->hide_neural_num] > max
			|| chromo_p->output_neural_p[m].power_p[chromo_p->hide_neural_num] < min )
		{
			chromo_p->output_neural_p[m].power_p[chromo_p->hide_neural_num] /= devide * 100; 
		}
	}*/

    return 1;
}

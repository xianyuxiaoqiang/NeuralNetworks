/*
** The neural
**
**/
#include "neuralnetwork.h"


void error_output( char *msg )
{
	FILE * fp = fopen("error_output.log", "a");
	if(fp)
	{
		fprintf(fp, msg);
		fprintf(fp, "\n");
		fclose(fp);
	}
}

void debug_output( char *msg )
{
	if( DEBUG_MSG )
	{
		FILE * fp = fopen("debug.log", "a");
		if(fp)
		{
			fprintf(fp, msg);
			fprintf(fp, "\n");
			fclose(fp);
		}
	}
}
/*
** sigmoid(x,p) = 1/( 1 + ( e ^ (-x/p) ) )
**/
double sigmoid( const double x, const double p )
{
	double result = 0;
	result = x / p * (-1);
	result = exp(result);
	return 1/(1+result);
}

/*
** xList   : the list of input
** xCount  : the number of input
** wList   : the list of weight
** wCount  : the number of weight
**/
double getActivation( double * xList, unsigned int xCount, double *wList, unsigned int wCount )
{
    double result = 0;
	unsigned int i;
	if( xList == NULL || wList == NULL || xCount != wCount )
	{
		error_output("getActivation : Invalid input!");
		return -1;
	}
	for( i = 0; i < xCount; i++ )
	{
		result += xList[i] * wList[i];
	}

	return result;    
}

double getOutput( const double activation )
{
	return sigmoid( activation , 100 );
}

double getOutputOfHideNeural( hide_neural_t *neural_p )
{
	double activation;
	double output;
	double * xList = malloc( sizeof(double) * neural_p->input_num );
	double * wList = malloc( sizeof(double) * neural_p->input_num );
	int i;
	if( xList == NULL || wList == NULL )
	{
		error_output("getOutputOfHideNeural : malloc error!");
		if( xList != NULL)free(xList);
		if( wList != NULL)free(wList);
		return 0;
	}
	for( i=0; i<neural_p->input_num; i++ )
	{
		xList[i] = neural_p->input_data_p[i];
		wList[i] = neural_p->power_p[i];
	}
	activation = getActivation( xList, neural_p->input_num, wList, neural_p->input_num );
	output = getOutput( activation );
	free(xList);
	free(wList);

	return output;
}
double getOutputOfNeuralnetwork( neuralnetwork_t * network_p)
{
	double activation;
	double output;
	int i,j;
	double * xList = malloc( sizeof(double) * (network_p->hide_neural_num + 1) ); // todo1 : more than one output
	double * wList = malloc( sizeof(double) * (network_p->hide_neural_num + 1) ); // todo1 : more than one output
	if( xList == NULL || wList == NULL )
	{
		error_output("getOutputOfHideNeural : malloc error!");
		if( xList != NULL)free(xList);
		if( wList != NULL)free(wList);
		return 0;
	}

	for( i=0; i<network_p->hide_neural_num; i++ )
	{
		xList[i] = getOutputOfHideNeural( &network_p->hide_neural_p[i] ); // the magic number
		wList[i] = network_p->output_neural_p[0].power_p[i]; // todo1 : more than one output  todo2 : more than one level of hide neural
	}
	xList[network_p->hide_neural_num] = -1;   // the special input
	wList[network_p->hide_neural_num] = network_p->output_neural_p[0].special_power; // todo1 : more than one output

	activation = getActivation( xList, network_p->hide_neural_num + 1, wList, network_p->hide_neural_num + 1 ); // todo1 : more than one output
	output = getOutput( activation );

	free(xList);
	free(wList);

	return output * 10;  // todo : to support more output. Current is 0 ~ 10
}

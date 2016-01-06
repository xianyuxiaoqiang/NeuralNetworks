#include "common.h"
#include "chromosome.h"


int init_chromo( chromo_t *chromo_p, unsigned int bits_per_gene, unsigned int gene_num )
{
	unsigned int size = ( bits_per_gene * gene_num + 8 ) / 8;
	chromo_p->bits_per_gene = 0;
	chromo_p->data_p = NULL;
	chromo_p->gene_num = 0;
	if( MAX_BITS_PER_GENE < bits_per_gene )
	{
		error_output( "Too long bits per gene." );
		return 1;
	}
	chromo_p->data_p = malloc( size );
	if( chromo_p->data_p == NULL )
	{
		error_output( "init_chomo: malloc fail." );
		return 1;
	}
	memset(chromo_p->data_p, 0, size);
	chromo_p->bits_per_gene = bits_per_gene;
	chromo_p->gene_num = gene_num;
	return 0;
}


int free_chromo( chromo_t *chromo_p )
{
	if( chromo_p->data_p != NULL )
	{
		free( chromo_p->data_p );
		chromo_p->data_p = NULL;
		chromo_p->bits_per_gene = 0;
		chromo_p->gene_num = 0;
	}
	return 0;
}


// this function only support no more than 1 byte
// the returned gene is starting from the first bit
char get_gene( chromo_t *chromo_p, unsigned int gene_index )
{
	char part1, part2;
	unsigned int level1,level2;
	if( gene_index >= chromo_p->gene_num )
	{
		error_output( "get_gene: out of range." );
		return 0;
	}
	// get first part of start byte
	level1 = gene_index * chromo_p->bits_per_gene;
	level2 = level1 - level1 / 8 * 8;
    level1 = level1 / 8;
	part1 = chromo_p->data_p[level1];
	part1 = part1 << level2;


	// get second part of continue byte
	if( level2 + chromo_p->bits_per_gene > 8 )
	{
		part2 = chromo_p->data_p[level1 + 1];
		// make sure start with 0
		part2 = part2 >> 1;
		part2 &= 0x7F;
		part2 = part2 >> ( 8 - level2 - 1 );
		// clear part2 of part1
		part1 &= 0xFF << level2;
		return part1 | part2;
	}


	return part1;
}




// this function only support no more than 1 byte
// the new_gene should start from the first bit and end with all "1"
int set_gene( chromo_t *chromo_p, unsigned int gene_index, char new_gene )
{
	char part1 = 0xFF, part2 = 0xFF;
	char tmp = 0xFF;
	unsigned int level1,level2;
	if( gene_index >= chromo_p->gene_num )
	{
		error_output( "set_gene: out of range." );
		return 1;
	}
	// set first part of start byte
	level1 = gene_index * chromo_p->bits_per_gene;
	level2 = level1 - level1 / 8 * 8;
    level1 = level1 / 8;
	part1 = (new_gene >> level2) | ( tmp << (8 - level2) ) ;
	// clear old value
	if( level2 + chromo_p->bits_per_gene < 8 )
	{
		tmp = 0xFF << (8 - level2);
		tmp |= 0x7F >> (level2 + chromo_p->bits_per_gene - 1);
		chromo_p->data_p[level1] &= tmp;
	}
	else
	{
		chromo_p->data_p[level1] &= ( tmp << (8 - level2) );
	}
	if( level2 > 0 )
	{
		// make sure start with 0
		part1 = new_gene >> 1;
		part1 &= 0x7F;
		part1 = part1 >> (level2-1);
		if( level2 + chromo_p->bits_per_gene < 8 )
		{
			tmp = 0xFF << (8 - level2);
			tmp |= 0x7F >> (level2 + chromo_p->bits_per_gene - 1);
			tmp = ~tmp;
			part1 &= tmp;
			chromo_p->data_p[level1] |= part1;
		}
		else
		{
			chromo_p->data_p[level1] |= part1;
		}
	}


	// set second part of continue byte
	if( level2 + chromo_p->bits_per_gene > 8 )
	{
		// clear old value
		tmp = 0x7F >> (level2 + chromo_p->bits_per_gene - 8 - 1);
		chromo_p->data_p[level1 + 1] &= tmp;


		part2 = (new_gene << ( 8 - level2 )) & (0xFF << ( 16 - level2 - chromo_p->bits_per_gene ));
		chromo_p->data_p[level1 + 1] |= part2;
	}
	return 0;
}

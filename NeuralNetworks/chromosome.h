#ifndef MY_CHROMOSOME_H_DEF
#define MY_CHROMOSOME_H_DEF

#define MAX_BITS_PER_GENE 8


typedef struct chromo_tag
{
	unsigned int bits_per_gene;
	unsigned int gene_num;
	char * data_p;
} chromo_t;


int init_chromo( chromo_t *chromo_p, unsigned int bits_per_gene, unsigned int gene_num );
int free_chromo( chromo_t *chromo_p );
char get_gene( chromo_t *chromo_p, unsigned int gene_index );
int set_gene( chromo_t *chromo_p, unsigned int gene_index, char new_gene );

#endif

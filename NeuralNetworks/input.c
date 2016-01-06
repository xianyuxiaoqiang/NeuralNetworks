#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// col_width : the width of one column, default is 2
input_data_t initInputData(char * fname, unsigned int col_width)
{
	FILE *inFile = NULL;
	char buffer[1025];
	size_t size = 0;
	unsigned int i;
	unsigned int temp_rownum = 0;
	unsigned int rawnum = 0;
	input_data_t indata;
	char *data_p = NULL;
	unsigned int data_index = 0;
	unsigned int temp_len = 0;

	buffer[1024] = '\0';
	indata.col_num = 0;
	indata.data_p = NULL;
	indata.row_num = 0;
	inFile = fopen(fname, "r");
	if( col_width > MAX_COL_WIDTH )
	{
        error_output("Too wide column.");
		goto ERROR_END;
	}
	if( inFile == NULL )
	{
        error_output("Fail to open file.");
		goto ERROR_END;
	}
    size = fread( buffer, 1, 1024, inFile );
	// looking for \n and init buffer
	for( i = 0; i < size; i++ )
	{
		if( buffer[i] == '\n' )
		{
			indata.col_num = i / col_width;
			temp_rownum = size / (i+1) + 1;
			data_p = malloc( (i+1) * temp_rownum );
			if( data_p == NULL )
			{
				error_output("Malloc fail.");
				goto ERROR_END;
			}
			break;
		}
	}
	if( indata.col_num == 0 )
	{
		error_output("Too long line.");
		goto ERROR_END;
	}
	while( size > 0 )
	{
		memcpy(data_p + data_index, buffer, size);
		// change all \n to \0 and check length
		for( i = 0; i < size; i++ )
		{
			if( buffer[i] == '\n' )
			{
				if( temp_len != indata.col_num * col_width )
				{
					if( temp_len == 0 )
					{
						// '\n' can be more than one
						continue;
					}
					error_output("Invalid line length.");
					goto ERROR_END;
				}
				data_p[ data_index + i ] = '\0';
				temp_len = 0;
				// realloc
				if( rawnum >= temp_rownum )
				{
					temp_rownum *= 2;
					data_p = realloc( data_p, (i+1) * temp_rownum );
					if( data_p == NULL )
					{
						error_output("Malloc fail.");
						goto ERROR_END;
					}
				}
				rawnum++;
			}
			else
			{
				temp_len++;
			}
		}
		data_index += size;

		size = fread( buffer, 1, 1024, inFile );
	}
	if( temp_len == indata.col_num * col_width )
	{
		// the last line
		data_p[ data_index ] = '\0';
		rawnum++;
	}

	// success
	indata.row_num = rawnum;
	indata.data_p = data_p;
	indata.col_width = col_width;
	if( inFile != NULL )
	{
		fclose(inFile);
		inFile = NULL;
	}
	return indata;

ERROR_END:
    if( data_p != NULL )
	{
		free(data_p);
		data_p = NULL;
	}
	if( inFile != NULL )
	{
		fclose(inFile);
		inFile = NULL;
	}
	return indata;
}

char *getAddress(input_data_t *indata_p, unsigned int col, unsigned int row)
{
	if( col >= indata_p->col_num || row >= indata_p->row_num )
	{
		return NULL;
	}
	return ( indata_p->data_p + (indata_p->col_num*indata_p->col_width + 1)*row + col*indata_p->col_width );
}
void freeInputData(input_data_t *indata_p)
{
	if( indata_p->data_p != NULL )
	{
		free( indata_p->data_p );
		indata_p->data_p = NULL;
		indata_p->col_num = 0;
		indata_p->col_width = 0;
		indata_p->row_num = 0;
	}
}
/* input_data_recognition_t initInputDataRecognition_toInt(char * fname)  // this function get 64 bit to 2 int. only get 2 input number
{
	input_data_recognition_t data;
	FILE *inFile = NULL;
	char buffer[1025];
	size_t size = 0;
	unsigned int i;
	unsigned int j;
	unsigned int byteCount = 0;
	unsigned int intCount = 0;
	int tmpdata = 0;
	char *tmp_p;

	inFile = fopen(fname, "r");
	if( inFile == NULL )
	{
        error_output("Fail to open file.");
		goto ERROR_END;
	}
    size = fread( buffer, 1, 1024, inFile );
	// todo : input more than 1024
	tmp_p = &buffer;
	for( i = 0; i < size; i++ )
	{
		if( buffer[i] == '\n' )
		{
			if( 16 != (&buffer[i] - tmp_p) )     // todo : more size of input.   Current is only 8*8
			{
				error_output("Invalid line length.");
				goto ERROR_END;
			}
			// todo : more codeset. Current is sjis. 
			// 0x81A0 : □(0)
			// 0x81A1 : ■(1)
			for( j=0; j<16; j+=2 )
			{
				if( tmp_p[j] == (char)0x81 && tmp_p[j+1] == (char)0xA0 )
				{
					((char *)&tmpdata)[byteCount] &= ( 0xFF << (8 - j/2) ) | (0x7F >> j/2);
				}
				else if( tmp_p[j] == (char)0x81 && tmp_p[j+1] == (char)0xA1 )
				{
					if( j == 0 )
					{
						((char *)&tmpdata)[byteCount] |= 0x80;
					}
					else
					{
						((char *)&tmpdata)[byteCount] |= 0x40 >> (j/2-1);
					}
				}
				else
				{
					error_output("Invalid input data.");
					goto ERROR_END;
				}
			}
			byteCount++;
			if( byteCount >= 4 )
			{
				if( intCount == 0 )
				{
					data.first = tmpdata;
					tmpdata = 0;
				}
				else
				{
					data.second = tmpdata;
					tmpdata = 0;
				}
				intCount++;
				byteCount = 0;
			}
			tmp_p = &buffer[i] + 1;
		}
	}


	// success
	if( inFile != NULL )
	{
		fclose(inFile);
		inFile = NULL;
	}
	return data;

ERROR_END:
	if( inFile != NULL )
	{
		fclose(inFile);
		inFile = NULL;
	}
	data.first = 0;
	data.second = 0;
	return data;
} */
input_data_recognition_t initInputDataRecognition(char * fname)
{
	input_data_recognition_t data;
	FILE *inFile = NULL;
	char buffer[1025];
	size_t size = 0;
	unsigned int i;
	unsigned int j;
	unsigned int byteCount = 0;
	unsigned int intCount = 0;
	int tmpdata = 0;
	char *tmp_p;

	inFile = fopen(fname, "r");
	if( inFile == NULL )
	{
        error_output("Fail to open file.");
		goto ERROR_END;
	}
    size = fread( buffer, 1, 1024, inFile );
	// todo : input more than 1024
	tmp_p = &buffer;
	for( i = 0; i < size; i++ )
	{
		if( buffer[i] == '\n' )
		{
			if( 16 != (&buffer[i] - tmp_p) )     // todo : more size of input.   Current is only 8*8
			{
				error_output("Invalid line length.");
				goto ERROR_END;
			}
			// todo : more codeset. Current is sjis. 
			// 0x81A0 : □(0)
			// 0x81A1 : ■(1)
			for( j=0; j<16; j+=2 )
			{
				if( tmp_p[j] == (char)0x81 && tmp_p[j+1] == (char)0xA0 )
				{
					data.data[byteCount] = 0;
					byteCount++;
				}
				else if( tmp_p[j] == (char)0x81 && tmp_p[j+1] == (char)0xA1 )
				{
					data.data[byteCount] = 1;
					byteCount++;
				}
				else
				{
					error_output("Invalid input data.");
					goto ERROR_END;
				}
				if( byteCount >= sizeof(data.data) )
				{
					break;
				}
			}
			tmp_p = &buffer[i] + 1;
		}
	}

	// success
	if( inFile != NULL )
	{
		fclose(inFile);
		inFile = NULL;
	}
	return data;

ERROR_END:
	if( inFile != NULL )
	{
		fclose(inFile);
		inFile = NULL;
	}
	memset( data.data, 0, sizeof(data.data));
	return data;
}

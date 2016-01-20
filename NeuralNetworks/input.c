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

#define PNG_BYTES_TO_CHECK 4

// translate all the png files in a folder to the input data
// if folder is NULL, get all these following folders' png files
// ./1  ./2  ./3 ... ./10
// the png files' expected value should be the folders' name.
// e.g.   ./1/1_1.png   : expected value = 1
//        ./10/10_9.png  : expected value = 10
// return the current index of the data_p (input number)
int initInputDataRecPNG_ex(char * folder, input_data_recognition_t *data_p, int start_index, int end_index)
{
	struct _finddata_t folder_data;
	struct _finddata_t file_data;
	long folder_handle;
	long file_handle;
	int folder_ret;
	int file_ret;
	int expect;
	int cur_index = start_index;
	if( folder == NULL )
	{
		folder_handle = _findfirst( "*", &folder_data );
		folder_ret = folder_handle > 0 ? 1 : -1;
		while( folder_ret >= 0 )
		{
			if( folder_data.attrib == _A_SUBDIR && atoi(folder_data.name) > 0)
			{
				debug_output(folder_data.name);
				debug_output("\n");
				// get one folder's input
				cur_index = initInputDataRecPNG_ex(folder_data.name, data_p, cur_index, end_index);
				if( cur_index > end_index )
				{
					return cur_index;
				}
			}
			folder_ret = _findnext( folder_handle, &folder_data );
		}
		_findclose( folder_handle ); 
	}
	else
	{
		_chdir(folder);
		file_handle = _findfirst( "*.png", &file_data );
		file_ret = file_handle > 0 ? 1 : -1;
		while( file_ret >= 0 )
		{
			if ( file_data.attrib != _A_SUBDIR )
			{
				debug_output(file_data.name);
				debug_output("\n");
				data_p[cur_index] = initInputDataRecPNG(file_data.name);
				expect = atoi(folder);
				if( expect <= 0 )
				{
					int i;
					char buf[11];
					for( i=0; i<strlen(file_data.name) && i<10; i++ )
					{
						if(file_data.name[i] > '9' || file_data.name[i] < '0')
						{
							break;
						}
						buf[i] = file_data.name[i];
					}
					buf[i] = 0;
					expect = atoi(buf);
				}
				data_p[cur_index].expcet = expect;
				cur_index++;
				if( cur_index > end_index )
				{
					return cur_index;
				}
			}
			file_ret = _findnext( file_handle, &file_data );
		}
		_findclose( file_handle ); 
		_chdir("..\\");
	}

	return cur_index;
}
input_data_recognition_t initInputDataRecPNG(char * fname)
{
	input_data_recognition_t data;
	FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;
    char buf[PNG_BYTES_TO_CHECK];
    int w, h, x, y, temp, color_type;
	int data_row_index,data_col_index;
	png_byte red,green,blue,alpha;

	data_row_index = data_col_index = 0;
	memset( &data, 0, sizeof(data) );
	if ((fp = fopen(fname, "rb")) == NULL)
	{
		error_output("initInputDataRecPNG: erro in fopen");
		return data;
	}

	/* Create and initialize the png_struct
	*/
	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );

	if (png_ptr == NULL)
	{
		error_output("initInputDataRecPNG: erro in png_create_read_struct");
		fclose(fp);
		return data;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		error_output("initInputDataRecPNG: erro in png_create_info_struct");
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return data;
	}

    //setjmp( png_jmpbuf(png_ptr) );
    temp = fread( buf, 1, PNG_BYTES_TO_CHECK, fp );
    if( temp < PNG_BYTES_TO_CHECK ) {
		error_output("initInputDataRecPNG: erro in fread");
        fclose(fp);
        png_destroy_read_struct( &png_ptr, &info_ptr, 0);
        return data;
    }  
    temp = png_sig_cmp( (png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK ); 
    if( temp != 0 ) {
		error_output("initInputDataRecPNG: erro in png_sig_cmp");
        fclose(fp);
        png_destroy_read_struct( &png_ptr, &info_ptr, 0);
        return data;
    }  
          
    rewind( fp );
    png_init_io( png_ptr, fp );
    png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0 );
    color_type = png_get_color_type( png_ptr, info_ptr );
    w = png_get_image_width( png_ptr, info_ptr );
    h = png_get_image_height( png_ptr, info_ptr );
    row_pointers = png_get_rows( png_ptr, info_ptr );
    switch( color_type ) {
    case PNG_COLOR_TYPE_RGB_ALPHA:
        for( y=0; y<h; ++y ) {
            for( x=0; x<w*4; ) {
                red = row_pointers[y][x++];
                green = row_pointers[y][x++];
                blue = row_pointers[y][x++];
                alpha = row_pointers[y][x++];
				RGBtoBool(w,h,x/4-1,y,red,green,blue,&data);
            }
        }
        break;

    case PNG_COLOR_TYPE_RGB:
        for( y=0; y<h; ++y ) {
            for( x=0; x<w*3; ) {
                red = row_pointers[y][x++];
                green = row_pointers[y][x++];
                blue = row_pointers[y][x++];
				RGBtoBool(w,h,x/3-1,y,red,green,blue,&data);
            }
        }
        break;
    default:
        fclose(fp);
        png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
        return data;
    }  
    png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
	if( DEBUG_MSG )
	{
		input_data_recognition_to_txt( &data, fname );
	}
    return data;
}

int RGBtoBool( int width, int hight, int x, int y, png_byte red, png_byte green, png_byte blue, input_data_recognition_t *data_p )
{
	int row_data, col_data;
	int result;
	double *tmp;
	row_data = y * DATA_INPUT_ROW_NUM / hight;
	if( row_data >= DATA_INPUT_ROW_NUM )
	{
		row_data = DATA_INPUT_ROW_NUM - 1;
	}
	col_data = x * DATA_INPUT_COL_NUM / width;
	if( col_data >= DATA_INPUT_COL_NUM )
	{
		col_data = DATA_INPUT_COL_NUM - 1;
	}
	if( red <= 10 && green <=10 && blue <= 10 )
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	tmp = &data_p->data[row_data * DATA_INPUT_COL_NUM + col_data];
	if( *tmp == 0 )
	{
		*tmp = result;
	}
	return 0;
}

int input_data_recognition_to_txt( input_data_recognition_t *data_p, char * fname )
{
	int i, j;
	FILE *fp;
	char filename[50];
	sprintf(filename, "%s.txt", fname);
	fp = fopen(filename, "w");
	if( fp != NULL )
	{
		for( i=0; i<DATA_INPUT_ROW_NUM; i++ )
		{
			for( j=0; j<DATA_INPUT_COL_NUM; j++ )
			{
				// sjis
				// 0x81A0 : □(0)
				// 0x81A1 : ■(1)
				fprintf(fp, "%c", (char)0x81 );
				if( data_p->data[ i * DATA_INPUT_COL_NUM + j ] == 0 )
				{
					fprintf( fp, "%c", (char)0xA0 );
				}
				else
				{
					fprintf( fp, "%c", (char)0xA1 );
				}
			}
			fprintf( fp, "\n" );
		}
		fclose(fp);
	}
}


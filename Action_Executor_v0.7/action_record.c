#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
//#include "qo_action.h"
#include "action_record.h"

void PrintAction(ActionRecord *ac)
{
	 printf("\naction time:%.1f", ac->time);
	 printf("\t id:%d", ac->id);
	 printf("\t soure:%d", ac->source);
	 printf("\t target:%d", ac->target);
	 printf("\t duration:%.1f\n", ac->duration);
}

ActionRecord * ReadOutputFile(char *filename,int *count)
{
	FILE *f;
	ActionRecord *ac;
	f = fopen(filename,"rb");
       size_t result;
	if( f == NULL)
	{
		fprintf(stderr,"Can not write file");
		return NULL;
	}
       fseek ( f, -sizeof(int) , SEEK_END );
	result=fread(count,1,sizeof(int),f);
	if(result != sizeof(int))
	{
		fputs ("Reading error\n",stderr); 
		goto bad_read;
	}
	if(*count < 0)
	{
		goto bad_read;
	}
	printf("\n count in read:%d\n",*count);

	ac = (ActionRecord *)malloc(sizeof(ActionRecord) * (*count));
	int i;
	i=0;
	ActionRecord temp;
	
	rewind(f);
	
	for(i=0; i<*count; i++)
	{
		//printf("\n i:%d\n",i);
		result=fread(&temp,1,sizeof(ActionRecord),f);
		if(result != sizeof(ActionRecord))
		{
			fputs ("Reading error\n",stderr); 
			goto bad_read;
		}
		memcpy(&ac[i],&temp,sizeof(ActionRecord));
		//PrintAction(&ac[i]);	

	}

	fclose(f);
	return ac;
bad_read:
	fclose(f);
	return NULL;
}

int WriteCountToFile(char *filename, const int *count)
{
	FILE *f;
	f = fopen(filename,"ab");
	if( f == NULL)
	{
		fprintf(stderr,"Can not write file");
		return -1;
	}
    	fwrite(count,1,sizeof(int),f);
	fclose(f);
	return 0;
}
int WriteToFile(const char *filename,ActionRecord *ac )
{
	FILE *f;
	f = fopen(filename,"ab");
	if( f == NULL)
	{
		fprintf(stderr,"Can not write file");
		return -1;
	}
	if(ac == NULL ){
		fprintf(stderr,"cannot write to file");
		goto bad_write_file;
	}
	//write the action records
	fwrite(ac, 1, sizeof(ActionRecord),f);
	
bad_write_file:
	fclose(f);
	return 0;
}

int CreateOutputFile(const char *filename)
{
	FILE *f;
	f = fopen(filename,"wb");
	if( f == NULL)
	{
		fprintf(stderr,"Can not write file");
		goto bad_create;
	}
bad_create:
	fclose(f);
	return 0;	
}


//-----------------------funtion of action record--------------------//
int CompareActionRecord(ActionRecord *A, ActionRecord *B)
{
	if( A->time < B->time) return -1;
	if (A->time > B->time) return 1;
	return 0;
}
void CopyActionRecord(ActionRecord *A,ActionRecord *B)
{
    
	A->time = B->time;
	A->id = B->id;
	A->source = B->source; 
	A->duration = B->duration;
	A->target = B->target;
}
void SwapActionRecord(ActionRecord *A, ActionRecord *B)
{
	ActionRecord temp;
	CopyActionRecord(&temp,A);
	CopyActionRecord(A,B);
	CopyActionRecord(B,&temp);
}
void SortActionArray(ActionRecord *ac,int count)
{
	int i,j;
	for (i =0; i < count -1 ; i++)
	{
		for( j = i +1;j<count;j++)
		{
			if(ac[i].time > ac[j].time)
			{
				SwapActionRecord(&ac[i],&ac[j]);
			}
		}
		
	}
}


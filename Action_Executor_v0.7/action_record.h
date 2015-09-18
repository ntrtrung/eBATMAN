#ifndef _ACTION_RECORD_H_
#define _ACTION_RECORD_H_

// node element and attribute names
#define XML_ACTION_SCENARIO            "action_scenario"
#define POINTOFTIME			    "point"
#define NODE				    "node"
#define NODE_ID			    "id"
#define ACTION			     "action"
#define ACTION_TYPE			    "action_id"
#define ACTION_DURATION			"duration"
#define ACTION_TARGET		 	"des_node"
#define TIME					"time"


struct action
{
	float time;
	int id;
	int source;
	float duration;
	int target;
};
typedef struct action ActionRecord;

void PrintAction(ActionRecord *ac);
ActionRecord * ReadOutputFile(char *filename,int *count);
int WriteCountToFile(char *filename, const int *count);
int WriteToFile(const char *filename,ActionRecord *ac );
int CreateOutputFile(const char *filename);
int CompareActionRecord(ActionRecord *A, ActionRecord *B);
void CopyActionRecord(ActionRecord *A,ActionRecord *B);
void SwapActionRecord(ActionRecord *A, ActionRecord *B);
void SortActionArray(ActionRecord *ac,int count);





#endif 


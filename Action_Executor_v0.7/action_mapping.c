#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "action_mapping.h"

char * look_up_action_file(action_mapping *am,int id)
{
	if(am == NULL) return NULL;
	action_mapping *temp;
	temp = am;
	while(temp!=NULL)
	{
		if(temp->action_id == id) return temp->filename;
		else
			temp=temp->next;
	}
	return NULL;
}
action_mapping * create_action_mapping_rc(int id,char *file)
{
	action_mapping *ac ;
	ac = (action_mapping*) malloc(sizeof(action_mapping));
	ac->filename= (char*) malloc (sizeof(char)*strlen(file)+1);
	memcpy(ac->filename,file,strlen(file));
	ac->action_id = id;
	return ac;
}

void print_action_mapping(action_mapping *ap)
{
	if(ap == NULL)
	{
		return;
	}
	printf("\naction id :%d",ap->action_id);
	printf("\nfile path:");
	puts(ap->filename);
}
action_mapping *read_action_mapping_file(char *filename,int *count)
{

  xmlNode *action_node,*child_node;
  xmlChar *action_id,*file_path;

  // --------------------------------------------------------------------------
  // Open XML document
  // --------------------------------------------------------------------------

  xmlDocPtr doc;
  doc = xmlParseFile(filename);

  if (doc == NULL) {
        printf("error: could not parse file file.xml\n");
		return 0;
	}
  
  // --------------------------------------------------------------------------
  // XML root.
  // --------------------------------------------------------------------------

  /*Get the root element node */
  xmlNode *root = NULL;
  root = xmlDocGetRootElement(doc);
  
  // --------------------------------------------------------------------------
  // Must have root element, a name and the name must be "action_mapping"
  // --------------------------------------------------------------------------
  
  if( !root || 
      !root->name ||
      xmlStrcmp(root->name,ACTION_MAPPING)) 
  {
     xmlFreeDoc(doc);
     fprintf(stderr,"Can not parse root node. Root node is action_scenario");
     goto bad_main;
  }
  
  // --------------------------------------------------------------------------
  //	Start parsing XML FILE to ActionMapping struct
  // --------------------------------------------------------------------------
 
  action_mapping *ap;
  action_mapping *first;
  first = create_action_mapping_rc(-1,"");
  ap = first;

 


  *count =0; 
  
  for(action_node = root->children; action_node != NULL; action_node = action_node->next)
  {
    
	if(action_node->type==XML_ELEMENT_NODE && !xmlStrcmp(action_node->name,(const xmlChar *)ACTION))
	{
		
		for(child_node = action_node->children;child_node != NULL; child_node = child_node->next)
		{
			//check node acition ID
			if(child_node->type==XML_ELEMENT_NODE && !xmlStrcmp(child_node->name,(const xmlChar *)ACTION_ID))
			{
					action_id = xmlNodeGetContent(child_node);
			}
			//check file path
			if(child_node->type==XML_ELEMENT_NODE && !xmlStrcmp(child_node->name,(const xmlChar *)FILE_PATH))
			{
				file_path = xmlNodeGetContent(child_node);
			}
		}
		ap->next = create_action_mapping_rc(atoi(action_id),file_path);
		print_action_mapping(ap->next);
		ap = ap->next;
		*count = *count +1;
		xmlFree(action_id);
		xmlFree(file_path);
		
		
	}
  }
  
 //free xml elemnent
	xmlFreeDoc(doc);
    xmlCleanupParser();
	return first->next;
	
 //bad cases
bad_main:
	printf("\n bad case");
      xmlFreeDoc(doc);
      xmlCleanupParser();
	return NULL ;
}


#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "action_record.h"
#include "create_action_binary.h"


int create_binary_from_xml_file(char *xml_file,char *binary_file)
{

  xmlNode *child_node,*point_node,*node_node,*action_node;
  char *pointoftime,*node_id;
  xmlChar *action_id,*action_duration,*action_target;

  // --------------------------------------------------------------------------
  // Open XML document
  // --------------------------------------------------------------------------

  xmlDocPtr doc;
  doc = xmlParseFile(xml_file);

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
  // Must have root element, a name and the name must be "action_scenario"
  // --------------------------------------------------------------------------
  
  if( !root || 
      !root->name ||
      xmlStrcmp(root->name,XML_ACTION_SCENARIO)) 
  {
     xmlFreeDoc(doc);
     fprintf(stderr,"Can not parse root node. Root node is action_scenario");
     goto bad_main;
  }
  
  // --------------------------------------------------------------------------
  //	Start parsing XML FILE to ActionRecord struct
  // --------------------------------------------------------------------------
  char *filename = binary_file;  
  CreateOutputFile(filename);
  ActionRecord var_action;
  
  if(CreateOutputFile(filename) == -1)
  {
	goto bad_main;
  }
  int count = 0;
 
  for(point_node = root->children; point_node != NULL; point_node = point_node->next)
  {
    //	check point of time node
	if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)POINTOFTIME))
	{
		// get propertise
		pointoftime = xmlGetProp(point_node,TIME);
		if(pointoftime)
		{
			var_action.time=atof(pointoftime);
		}
		xmlFree(pointoftime);
		
		// check node of node
		for(node_node = point_node->children; node_node != NULL; node_node = node_node->next)
		{
			if(node_node->type==XML_ELEMENT_NODE && !xmlStrcmp(node_node->name,(const xmlChar *)NODE))
			{    
				// get node id 
				node_id = xmlGetProp(node_node,NODE_ID);
				if(node_id) 
				{
					var_action.source=atoi(node_id);
					
				}
				xmlFree(node_id);
				// check action node
				for(action_node = node_node->children;action_node != NULL; action_node = action_node->next)
				{
					if(action_node->type==XML_ELEMENT_NODE && !xmlStrcmp(action_node->name,(const xmlChar *)ACTION))
					{
						// get all information of action
				        for(child_node = action_node->children; child_node != NULL; child_node = child_node->next)
						{
							// action type 
							if(child_node->type==XML_ELEMENT_NODE && !xmlStrcmp(child_node->name,(const xmlChar *)ACTION_TYPE))
							{
								action_id = xmlNodeGetContent(child_node);
								if(action_id)
								{
									var_action.id=atoi(action_id);
								}
								xmlFree(action_id);
							}
							// action duration
							if(child_node->type==XML_ELEMENT_NODE && !xmlStrcmp(child_node->name,(const xmlChar *)ACTION_DURATION))
							{
								action_duration = xmlNodeGetContent(child_node);
								if(action_duration) 
								{
									var_action.duration=atof(action_duration);
								}

								xmlFree(action_duration);
							}
							// action target
							if(child_node->type==XML_ELEMENT_NODE && !xmlStrcmp(child_node->name,(const xmlChar *)ACTION_TARGET))
							{
								action_target = xmlNodeGetContent(child_node);
								if(action_target)
								{ 
									var_action.target=atoi(action_target);
								}
								xmlFree(action_target);
							}
						}
						
						//increase count 
						count++;
						//write action record to file
						//printf("\ncount : %d\n",count);
						if( WriteToFile(filename,&var_action) == -1)
						{
							fprintf(stderr,"\n Cannot write output file\n");
							goto bad_main;
						}
						PrintAction(&var_action);
					}
					
				}
				
			}
			
		}
	  		
		
	}
	}
	 if( WriteCountToFile(filename,&count)==-1)
	{
		fprintf(stderr,"\n Cannot write output file\n");
		goto bad_main;
	}
	ReadOutputFile(filename,&count);
  /*
	   *Free the global variables that may
	   *have been allocated by the parser.
	   */
      xmlFreeDoc(doc);
      xmlCleanupParser();

	return 0;

bad_main:
	printf("\nbad case");
      xmlFreeDoc(doc);
      xmlCleanupParser();

	return -1;
}

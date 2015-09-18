#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "read_config.h"

int read_config_file(be_configuration *rc,char *filename)
{

  xmlNode *child_node,*point_node;
  xmlChar *xml_control_interface,*xml_ex_interface;

  // --------------------------------------------------------------------------
  // Open XML document
  // --------------------------------------------------------------------------

  xmlDocPtr doc;
  doc = xmlParseFile(filename);

  if (doc == NULL) {
        printf("error: could not parse file file.xml\n");
		return -1;
	}
  
  // --------------------------------------------------------------------------
  // XML root.
  // --------------------------------------------------------------------------

  /*Get the root element node */
  xmlNode *root = NULL;
  root = xmlDocGetRootElement(doc);
  
  
  if( !root || !root->name ||xmlStrcmp(root->name,"Broad_Envi")) 
  {
     xmlFreeDoc(doc);
     fprintf(stderr,"Can not parse root node. Root node is action_scenario");
     goto bad_main;
  }
  
  // --------------------------------------------------------------------------
  //	Start parsing XML FILE to get Report Collector Information
  // --------------------------------------------------------------------------
   
  for(point_node = root->children; point_node != NULL; point_node = point_node->next)
  {
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"experiment_interface"))
		{
			 xml_ex_interface = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"control_interface"))
		{
			 xml_control_interface = xmlNodeGetContent(point_node);					
		}

  }
	rc->experiment_interface = (char*)malloc(strlen(xml_ex_interface)+1);
	memcpy(rc->experiment_interface,xml_ex_interface,strlen(xml_ex_interface));
	rc->experiment_interface[strlen(xml_ex_interface)]='\0';
	
	rc->control_interface = (char*)malloc(strlen(xml_control_interface)+1);
	memcpy(rc->control_interface,xml_control_interface,strlen(xml_control_interface));
	rc->control_interface[strlen(xml_control_interface)]='\0';


	xmlFree(xml_control_interface);
	xmlFree(xml_ex_interface);
    xmlFreeDoc(doc);
    xmlCleanupParser();
	return 0;

bad_main:
      printf("\nbad case");
      xmlFreeDoc(doc);
      xmlCleanupParser();

	return -1;
}


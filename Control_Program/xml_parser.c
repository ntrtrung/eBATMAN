#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "xml_parser.h"

void print_program_list(program_list *cf)
{
	printf("\n setting file:%s",cf->qomet);
	printf("\n broad_envi:%s",cf->broad_envi);
	printf("\n do_action:%s",cf->do_action);
	printf("\n olsrd:%s",cf->olsrd);
}
int read_process_list_xml(program_list *list,char *filename)
{
  
  xmlNode *child_node,*point_node;
  xmlChar *xml_qomet,*xml_broad_envi,*xml_do_action,*xml_olsrd;

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
  
  
  if( !root || !root->name ||xmlStrcmp(root->name,"PROGRAM_LIST")) 
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
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"QOMET"))
		{
			 xml_qomet = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"BROAD_ENVI"))
		{
			 xml_broad_envi = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"DO_ACTION"))
		{
			 xml_do_action = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"OLSRD"))
		{
			 xml_olsrd = xmlNodeGetContent(point_node);
					
		}
		

  }
	memcpy(list->qomet,xml_qomet,strlen(xml_qomet));
	list->qomet[strlen(xml_qomet)]='\0';
	
	memcpy(list->broad_envi,xml_broad_envi,strlen(xml_broad_envi));
	list->broad_envi[strlen(xml_broad_envi)]='\0';
	
	memcpy(list->do_action,xml_do_action,strlen(xml_do_action));
	list->do_action[strlen(xml_do_action)]='\0';

	memcpy(list->olsrd,xml_olsrd,strlen(xml_olsrd));
	list->olsrd[strlen(xml_olsrd)]='\0';
	
	xmlFree(xml_qomet);
	xmlFree(xml_broad_envi);
	xmlFree(xml_do_action);
	xmlFree(xml_olsrd);
       xmlFreeDoc(doc);
       xmlCleanupParser();
	return 0;

bad_main:
      printf("\nbad case");
      xmlFreeDoc(doc);
      xmlCleanupParser();

	return -1;
}


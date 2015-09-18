#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "read_config.h"

int read_config_file(rc_configuration *rc,char *filename)
{

  xmlNode *child_node,*point_node;
  xmlChar *xml_rc_interface,*xml_setting_file,*xml_data_server_list,*xml_log_file_path;

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
  
  // --------------------------------------------------------------------------
  // Must have root element, a name and the name must be "action_scenario"
  // --------------------------------------------------------------------------
  
  if( !root || !root->name ||xmlStrcmp(root->name,"Report_Collector")) 
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
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"Report_Interface"))
		{
			 xml_rc_interface = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"QOMET_setting_file"))
		{
			 xml_setting_file = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"Data_Server_List"))
		{
			 xml_data_server_list = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"Log_File_Path"))
		{
			 xml_log_file_path = xmlNodeGetContent(point_node);
					
		}
  }
	rc->rc_interface = (char*)malloc(strlen(xml_rc_interface)+1);
	memcpy(rc->rc_interface,xml_rc_interface,strlen(xml_rc_interface));
	rc->rc_interface[strlen(xml_rc_interface)]='\0';
	
	rc->setting_file = (char*)malloc(strlen(xml_setting_file)+1);
	memcpy(rc->setting_file,xml_setting_file,strlen(xml_setting_file));
	rc->setting_file[strlen(xml_setting_file)]='\0';
	
	rc->data_server_list = (char*)malloc(strlen(xml_data_server_list)+1);
	memcpy(rc->data_server_list,xml_data_server_list,strlen(xml_data_server_list));
	rc->data_server_list[strlen(xml_data_server_list)]='\0';
	
	rc->log_file_path = (char*)malloc(strlen(xml_log_file_path)+1);
	memcpy(rc->log_file_path,xml_log_file_path,strlen(xml_log_file_path));
	rc->log_file_path[strlen(xml_log_file_path)]='\0';
	
	printf("\n%s \n%s \n%s \n%s",rc->rc_interface,rc->setting_file,rc->data_server_list,rc->log_file_path);

	xmlFree(xml_rc_interface);
	xmlFree(xml_setting_file);
	xmlFree(xml_data_server_list);
	xmlFree(xml_log_file_path);
    xmlFreeDoc(doc);
    xmlCleanupParser();
	return 0;

bad_main:
	printf("\nbad case");
      xmlFreeDoc(doc);
      xmlCleanupParser();

	return -1;
}


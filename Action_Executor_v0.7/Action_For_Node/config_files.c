#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "config_files.h"

void print_config_files(config_files *cf)
{
	printf("\n setting file:%s",cf->setting_file);
	printf("\n neighbor_table_bin:%s",cf->neighbor_table_bin);
	printf("\n be_config:%s",cf->be_config);
	printf("\n ip_to_ip:%s",cf->ip_to_ip);
	printf("\n action_record:%s",cf->action_record);
	printf("\n action_mapping:%s",cf->action_mapping);
	printf("\n action:%s",cf->action);
	printf("\n qomet scenario binary:%s",cf->scenario_file_bin);
}
int read_config_files(config_files *cf)
{
  char *filename = CONFIG_FILES_PATH;
  
  xmlNode *child_node,*point_node;
  xmlChar *xml_setting_file,*xml_neighbor_table_bin,*xml_be_config,*xml_ip_to_ip,*xml_action_record,*xml_action_mapping,*xml_action,*xml_scenario_file_bin;

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
  
  
  if( !root || !root->name ||xmlStrcmp(root->name,"CONFIG_FILES")) 
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
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"SETTING_FILE"))
		{
			 xml_setting_file = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"NEIGHBOR_TABLE_BIN"))
		{
			 xml_neighbor_table_bin = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"BE_CONFIG"))
		{
			 xml_be_config = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"IP_TO_IP"))
		{
			 xml_ip_to_ip = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"ACTION_RECORD"))
		{
			 xml_action_record = xmlNodeGetContent(point_node);
					
		}if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"ACTION_MAPPING"))
		{
			 xml_action_mapping = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"ACTION"))
		{
			 xml_action = xmlNodeGetContent(point_node);
					
		}
		if(point_node->type==XML_ELEMENT_NODE && !xmlStrcmp(point_node->name,(const xmlChar *)"QOMET_BIN"))
		{
			 xml_scenario_file_bin = xmlNodeGetContent(point_node);
		}
	

  }
	memcpy(cf->setting_file,xml_setting_file,strlen(xml_setting_file));
	cf->setting_file[strlen(xml_setting_file)]='\0';
	
	memcpy(cf->neighbor_table_bin,xml_neighbor_table_bin,strlen(xml_neighbor_table_bin));
	cf->neighbor_table_bin[strlen(xml_neighbor_table_bin)]='\0';
	
	memcpy(cf->be_config,xml_be_config,strlen(xml_be_config));
	cf->be_config[strlen(xml_be_config)]='\0';

	memcpy(cf->ip_to_ip,xml_ip_to_ip,strlen(xml_ip_to_ip));
	cf->ip_to_ip[strlen(xml_ip_to_ip)]='\0';
	
	memcpy(cf->action_record,xml_action_record,strlen(xml_action_record));
	cf->action_record[strlen(xml_action_record)]='\0';
	
	memcpy(cf->action_mapping,xml_action_mapping,strlen(xml_action_mapping));
	cf->action_mapping[strlen(xml_action_mapping)]='\0';
	
	memcpy(cf->action,xml_action,strlen(xml_action));
	cf->action[strlen(xml_action)]='\0';
	
	memcpy(cf->scenario_file_bin,xml_scenario_file_bin,strlen(xml_scenario_file_bin));
	cf->scenario_file_bin[strlen(xml_scenario_file_bin)]='\0';
	
	xmlFree(xml_setting_file);
	xmlFree(xml_neighbor_table_bin);
	xmlFree(xml_be_config);
	xmlFree(xml_ip_to_ip);
	xmlFree(xml_action_record);
	xmlFree(xml_action_mapping);
	xmlFree(xml_action);
	xmlFree(xml_scenario_file_bin);
    xmlFreeDoc(doc);
    xmlCleanupParser();
	return 0;

bad_main:
      printf("\nbad case");
      xmlFreeDoc(doc);
      xmlCleanupParser();

	return -1;
}


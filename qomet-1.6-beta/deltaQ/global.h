
/*
 * Copyright (c) 2006-2010 The StarBED Project  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/************************************************************************
 *
 * QOMET Emulator Implementation
 *
 * File name: global.h
 * Function:  Header file with global defines
 *
 * Author: Razvan Beuran
 *
 *   $Revision: 180 $
 *   $LastChangedDate: 2010-03-26 09:47:53 +0900 (Fri, 26 Mar 2010) $
 *   $LastChangedBy: razvan $
 *
 ***********************************************************************/


#ifndef __GLOBAL_H
#define __GLOBAL_H


/////////////////////////////////////////////
// Basic constants
/////////////////////////////////////////////

// BOOLEAN
#define TRUE                            1
#define FALSE                           0

#define SUCCESS                         0
#define ERROR                           -1

#ifndef MAX_STRING
#define MAX_STRING                      256
#endif


/////////////////////////////////////////////
// Definitions of the main structures
// (used to avoid circular references)
/////////////////////////////////////////////

struct node_class;
struct object_class;
struct environment_class;
struct motion_class;
struct connection_class;
struct scenario_class;
struct xml_scenario_class;
struct xml_jpgis_class;

#endif

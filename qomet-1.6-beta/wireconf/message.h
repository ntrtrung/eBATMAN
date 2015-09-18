
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
 * File name: message.h
 * Function: Header file for the messages in the wireconf library
 *
 * Authors: Razvan Beuran
 *
 *   $Revision: 128 $
 *   $LastChangedDate: 2009-02-06 10:21:50 +0900 (Fri, 06 Feb 2009) $
 *   $LastChangedBy: razvan $
 *
 ***********************************************************************/


#ifdef MESSAGE_WARNING
#define WARNING(message...) do {                                          \
  fprintf(stderr, "wireconf WARNING: %s, line %d: ", __FILE__, __LINE__); \
  fprintf(stderr, message); fprintf(stderr,"\n");                         \
} while(0)
#else
#define WARNING(message...)	/* message */
#endif

#ifdef MESSAGE_DEBUG
#define DEBUG(message...) do {                                            \
  fprintf(stdout, "wireconf DEBUG: %s, line %d: ", __FILE__, __LINE__);   \
  fprintf(stdout, message); fprintf(stdout,"\n");                         \
} while(0)
#else
#define DEBUG(message...)	/* message */
#endif

#ifdef MESSAGE_INFO
#define INFO(message...) do {                                            \
  fprintf(stdout, message); fprintf(stdout,"\n");                        \
} while(0)
#else
#define INFO(message...)	/* message */
#endif

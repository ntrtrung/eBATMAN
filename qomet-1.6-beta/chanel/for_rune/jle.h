/*
 * Copyright (c) 2010 NAKATA, Junya (jnakata@nict.go.jp)
 * All rights reserved.
 *
 * $Id: jle.h,v 1.1 2010/02/04 10:05:03 jnakata Exp $
 */

#ifndef _JLE_H_
#define _JLE_H_

#define PHYHDRLEN 6

typedef struct {
	uint32_t len;
	uint8_t data[127 + PHYHDRLEN];
} jleCondData;

typedef struct {
	condPacket packet;
	int num;
} jleCondPacket;

typedef struct {
	int gsid;
	int fd;
	char path[BUFSIZ]; 
	ore_hw_t *hw[1];
	ore_st_t *st[1];
	jleCondPacket response;
	pthread_mutex_t mutex;
	int plen;
	uint8_t buff[127 + PHYHDRLEN];
} jlecommonstruct;

#endif /* _JLE_H_ */


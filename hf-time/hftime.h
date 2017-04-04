/*
 * hftime.h
 *
 *  Created on: Apr 14, 2015
 *      Author: holm
 */

#ifndef HFTIME_H_
#define HFTIME_H_


#define _XOPEN_SOURCE 700

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


time_t hf_isoutc2epoch(char* isodate);

int hf_epoch2isoutc(time_t epoch, char* buffer, size_t buf_sz);

void hf_free(void);

#endif /* HFTIME_H_ */

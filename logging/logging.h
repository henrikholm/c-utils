/*
 * logging.h
 *
 *  Created on: Apr 14, 2014
 *      Author: holm
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	void (*init)(void);
	void (*close)(void);
	void (*info)(const char* format, ...);
	void (*warning)(const char* format, ...);
	void (*error)(const char* format, ...);
} logging_ns;

extern logging_ns const logging_methods;



#endif /* LOGGING_H_ */

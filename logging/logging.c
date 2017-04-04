/*
 * logging.c
 *
 *  Created on: Apr 14, 2014
 *      Author: holm
 */

#include "logging.h"

void __log(const char* prefix, int severity, const char* format, va_list args){

	int format_sz = strlen(format);
	int prefix_sz = strlen(prefix);

	char* buffer = (char*)malloc( sizeof(char)*(prefix_sz + format_sz + 2) );

	sprintf(buffer, "%s %s", prefix, format);

#ifdef STANDALONE

	vprintf(buffer,args);

#else

	vsyslog(severity, format, args);

#endif

	free(buffer);

}

void logging_info(const char* format, ...){
	va_list args;
	va_start(args,format);

	__log("INFO:", LOG_INFO, format, args);

	va_end(args);

}
void logging_warning(const char* format, ...){
	va_list args;
	va_start(args,format);

	__log("WARNING:", LOG_WARNING, format, args);

	va_end(args);

}

void logging_error(const char* format, ...){
	va_list args;
	va_start(args,format);

	__log("ERROR:", LOG_ERR, format, args);

	va_end(args);

}

void logging_init(void){

#ifndef WITH_GFX
	openlog ("svoyopt", LOG_CONS | LOG_NDELAY, LOG_LOCAL2);
#endif

}

void logging_free(void){

#ifndef WITH_GFX
	closelog();
#endif

}

logging_ns const logging_methods = {
		.init = &logging_init,
		.close = &logging_free,
		.info = &logging_info,
		.warning = &logging_warning,
		.error = &logging_error
};

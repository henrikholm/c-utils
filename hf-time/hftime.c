/*
 * hftime.c
 *
 *  Created on: Apr 14, 2015
 *      Author: holm
 */

#include "hftime.h"

#define MIN_DATE_SZ 19

#define DASH_1_MARK 4
#define DASH_2_MARK 7
#define T_MARK 10
#define COLON_1_MARK 13
#define COLON_2_MARK 16
#define END_MARK 19

#define DATE_FORMAT "%Y-%m-%dT%H:%M:%S"

#define YEAR_MIN 70
#define YEAR_MAX 133 // 2033 - 1900

#define MONTHS_IN_YEAR 12

#define SECONDS_IN_YEAR 31536000
#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

/*
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
*/
#define likely(x) (x)
#define unlikely(x) (x)


typedef struct tm Hftm, *hftm;

// Table to hold months in a year
typedef struct {
	time_t months[MONTHS_IN_YEAR+1];
} Year, *year;

year* timetable = NULL;

// Malloc the memory for all years
// but do not init all years, only the ones we use
void hf_init(void){
	if(timetable == NULL){
		timetable = malloc(sizeof(year)*YEAR_MAX);
		for(int i = 0; i < YEAR_MAX; i++){
			timetable[i] = NULL;
		}
	}
}

// Release all years that are not null
void hf_free(void){
	if(timetable != NULL){
		for(int i = 0; i < YEAR_MAX; i++){
			if(timetable[i] != NULL){
				free(timetable[i]);
			}
		}
		free(timetable);
	}
}

// Set/unset the environment timezone. We set UTC to get the right dates
// from the system call
static char* _set_utc(void){
	char *tz = getenv("TZ");
	if (tz){
		tz = strdup(tz);
	}
	setenv("TZ", "", 1);
	tzset();
	return tz;
}
static void _unset_utc(char* tz){
	if (tz) {
		setenv("TZ", tz, 1);
		free(tz);
	} else {
		unsetenv("TZ");
	}
	tzset();
}


// Populate the year table with the starting second for each
// month of each year
static void _hf_init_year(uint32_t year_index){

	timetable[year_index] = malloc(sizeof(Year));

	Hftm tp;
	tp.tm_year = (int32_t)(year_index + YEAR_MIN);
	tp.tm_mday = 1;

	tp.tm_hour = 0;
	tp.tm_min = 0;
	tp.tm_sec = 0;
	tp.tm_isdst = 0;

	char* tz = _set_utc();

	for(int i = 0; i < (MONTHS_IN_YEAR+1); i++){
		tp.tm_mon = i;
		time_t starttime = mktime(&tp);

		timetable[year_index]->months[i] = starttime;

	}

	_unset_utc(tz);

}

// Sanity check the integer representation of each field
static int _check_valid(hftm timestamp){

	int is_valid = 1;

	if(unlikely( (timestamp->tm_year < YEAR_MIN) || (timestamp->tm_year > YEAR_MAX) )){
		is_valid = 0;
	}

	if(unlikely( (timestamp->tm_mon < 0) || (timestamp->tm_mon > 11) )){
		is_valid = 0;
	}

	if(unlikely( (timestamp->tm_mday < 1) || (timestamp->tm_mday > 31) )){
		is_valid = 0;
	}

	if(unlikely( (timestamp->tm_hour < 0) || (timestamp->tm_hour > 23) )){
		is_valid = 0;
	}

	if(unlikely( (timestamp->tm_min < 0) || (timestamp->tm_min > 59) )){
		is_valid = 0;
	}

	if(unlikely( (timestamp->tm_sec < 0) || (timestamp->tm_sec > 60) )){
		is_valid = 0;
	}


	return is_valid;
}


// Calculate the epoch value from a timestamp
time_t hf_time2epoch(hftm timestamp){

	if(unlikely( (_check_valid(timestamp) == 0) )){
		printf("ERROR: Timestamp invalid!\n");
		return -1;
	}

	if(unlikely(timetable == NULL)){
		hf_init();
	}

	// Get the starting point second for the month
	uint32_t year_index = (uint32_t)(timestamp->tm_year - YEAR_MIN);
	if(timetable[year_index] == NULL){
		_hf_init_year(year_index);
	}
	time_t epoch = timetable[year_index]->months[timestamp->tm_mon];

	// Add seconds in the number of days, hours, minutes

	// Day seconds
	epoch += (timestamp->tm_mday-1)*SECONDS_IN_DAY;

	// Hour seconds
	epoch += (timestamp->tm_hour)*SECONDS_IN_HOUR;

	// Minute seconds
	epoch += (timestamp->tm_min)*SECONDS_IN_MINUTE;

	epoch += timestamp->tm_sec;

	// Make sure the calculated value isn't larger than the
	// starting point of the next month
	if(epoch >= timetable[year_index]->months[timestamp->tm_mon+1]){
		printf("ERROR: Timestamp invalid!\n");
		return -1;
	}

	return epoch;

}

// Get the remainder divided by 'divisor'
static uint32_t _int_division(time_t epoch, time_t* start, uint32_t divisor){

	// Epoch is always larger than 'start'
	// 'ret' is the difference expressed in unit of the divisor
	// i.e. if 'divisor' is seconds in an hour then 'ret' is the difference
	// in hours
	uint32_t ret = (uint32_t)((epoch - *start)/(divisor));

	// Add the integer difference, for use the next time
	*start += ret*divisor;

	return ret;

}

int hf_epoch2isoutc(time_t epoch, char* buffer, size_t buf_sz){

	// There must be something wrong if we want dates before 1970
	if(epoch < 0){
		return -1;
	}

	// Make sure we have big enough output buffer
	if(buf_sz < MIN_DATE_SZ){
		return -1;
	}

	// Get the year starting point
	uint32_t year_index = (uint32_t)(epoch / SECONDS_IN_YEAR);
	if(timetable[year_index] == NULL){
		_hf_init_year(year_index);
	}
	time_t year_start = timetable[year_index]->months[0];

	// If the starting point is actually later, go one year back
	// This can happend due to leap days/seconds/etc
	if(year_start > epoch){
		year_index--;
		if(timetable[year_index] == NULL){
			_hf_init_year(year_index);
		}
		year_start = timetable[year_index]->months[0];
	}

	// Assume all months have 31 days, get estimated month starting point
	uint32_t month_index = (uint32_t)((epoch - year_start)/(SECONDS_IN_DAY*31));
	time_t month_end = (timetable[year_index]->months[month_index+1] - 1);

	// If we overshoot then back up
	if(epoch > month_end){
		month_index++;
	}
	time_t diff = timetable[year_index]->months[month_index];

	uint32_t day_index = _int_division(epoch, &diff, SECONDS_IN_DAY);

	uint32_t hour_index = _int_division(epoch, &diff, SECONDS_IN_HOUR);

	uint32_t minute_index = _int_division(epoch, &diff, SECONDS_IN_MINUTE);

	uint32_t seconds = (uint32_t)(epoch - diff);

	sprintf(buffer, "%d-%02d-%02dT%02d:%02d:%02dZ", (year_index + 1970), month_index+1, day_index+1, hour_index, minute_index, seconds);

	return 0;
}


time_t hf_isoutc2epoch(char* isodate){

	//
	// Format sanity checks
	//

	size_t d_sz = strlen(isodate);
	if(unlikely(d_sz < MIN_DATE_SZ)){
		printf("ERROR: Date string is too short!\n");
		return -1;
	}

	char* isocopy = strdup(isodate);

	int format_error = 0;
	if(unlikely(isocopy[DASH_1_MARK] != '-')){
		format_error = 1;
	}
	if(unlikely(isocopy[DASH_2_MARK] != '-')){
		format_error = 1;
	}
	if(unlikely(isocopy[T_MARK] != 'T')){
		format_error = 1;
	}
	if(unlikely(isocopy[COLON_1_MARK] != ':')){
		format_error = 1;
	}
	if(unlikely(isocopy[COLON_2_MARK] != ':')){
		format_error = 1;
	}
	if(unlikely(format_error)){
		printf("ERROR: Wrong format. Is '%s', should be '%s'\n", isodate, DATE_FORMAT);
		return -1;
	}

	// Format checks complete


	// Write string terminators
	isocopy[DASH_1_MARK] = 0;
	isocopy[DASH_2_MARK] = 0;
	isocopy[T_MARK] = 0;
	isocopy[COLON_1_MARK] = 0;
	isocopy[COLON_2_MARK] = 0;
	isocopy[END_MARK] = 0;



	// Parse every block between terminators as numbers
	// The date has 6 numbers
	// Increment the 10base for every position in the block
	//
	// "1970-01-01T00:00:00"
	// -> "1970<NULL>01<NULL>01<NULL>00<NULL>00<NULL>00<NULL>"
	// -> {1970, 1, 1, 0, 0, 0}
	//
	char* sptr = isocopy;
	char* eptr = (isocopy + MIN_DATE_SZ -1);
	int values[] = {0, 0, 0, 0, 0, 0};
	int base = 1;
	int value_index = 5;

	for(;eptr >= sptr; eptr--){
		if(*eptr == 0){
			base = 1;
			value_index--;
			continue;
		}
		if(unlikely(*eptr < '0' || *eptr > '9')){
			format_error = 1;
			break;
		}

		// Get number and multiply by position of base 10
		int v = (*eptr) - '0';
		values[value_index] += (v*base);
		base *= 10;
	}

	free(isocopy);

	if(unlikely(format_error)){
		printf("ERROR: Wrong format. Is '%s', should be '%s'\n", isodate, DATE_FORMAT);
		return -1;
	}


	Hftm dt;

	dt.tm_year = values[0] - 1900;
	dt.tm_mon = values[1] - 1;
	dt.tm_mday = values[2];
	dt.tm_hour = values[3];
	dt.tm_min = values[4];
	dt.tm_sec = values[5];


	return hf_time2epoch(&dt);

}



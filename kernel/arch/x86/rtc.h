#ifndef RTC_H
#define RTC_H


typedef struct
{

    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
    unsigned int second;

} RTC_TIME;


void rtc_read_time(RTC_TIME* time);


#endif

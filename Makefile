all: schedule

schedule: schedule.c
	gcc -pthread -Wall -g -o schedule  schedule.c -lm 

clean:
	rm -fr schedule *dSYM

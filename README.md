# Cpu_Schedular
 Summary:
 A C program that situmulates cpu scheduling with multi-threading and synchranization.
 
 Detailed Description:
 The program creates user given number of threads to create randomly generated cpu bursts which are then handed to the schedular thread of the program. The user can choose
 different algorithms to handle the threads which are FCFS (first come first serve), SJF (shortest job first), PRIO (prioritized threads) and VRUNTIME. The user can also 
 set bounds to the generation amounts and lenghts. The program can also create files which are filled with randomly generated bursts to be used later again or can take those 
 bursts as input which are pre-generated. The following are some example inputs:
 
 ./schedule <N> <Bcount> <minB> <avgB> <minA> <avgA> <ALG> where N is the number of threads, Bcount is the amount of bursts each thread will create, minB is the minimum
 value for each burst (if the value generated less than this value, the burst is generated again), avgB is the average of burst amounts (this value is used in 
 generating the bursts according to exponential random distribution), minA is the minimum arrival time of consecutive burts, avgA is the average value of arrival
 time of consecutive burts.
 
./schedule 8 10 100 200 100 150 VRUNTIME (generates bursts according to the parameters)
./schedule 8 10 100 200 100 150 VRUNTIME readFiles (generates bursts according to the parameter and writes each threads burst to its specific file, 
readfiles-1.txt, readfiles-2.txt and so on untill readfiles-n.txt)
./schedule 5 FCFS -f infile (reads bursts from file, infile-1.txt and so on)

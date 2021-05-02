#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>


#define SLEEP_TIME 1000

struct rq *head;
int bIndex = 0;
int n;
int bCount;
pthread_mutex_t lock;
pthread_cond_t bGenerated;
struct timeval current_time;
int fileinput, fileoutput;
char inprefix[50];
int done;

struct tInfo {
	int avgA;			
	int minA;			
	int avgB;
	int minB;			
	int bCount;			
	int index;
	int runtime;
};  

typedef struct rq {
	int threadIndex;
	int burstIndex;
	int length;
	int timeCreated;
	struct node* next;
}rq;

int expo(int avg){
    double u;
    double lambda = 1 / (double)avg;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}


void add(struct tInfo *burst, int bAmount);


void *generateBurst(void *param){

        int arrival, bAmount;

	printf("My id is: %d\n", ((struct tInfo *) param)->index);

        if(fileinput) {
          char filename[200];
          sprintf(filename, "%s-%d.txt", inprefix, ((struct tInfo *) param)->index);
          FILE* thread_file = fopen(filename, "r");
          while(!feof(thread_file)) {
            fscanf(thread_file, "%d %d\n", &arrival, &bAmount); 
            printf("adding %d %d\n", arrival, bAmount);
            add(((struct tInfo *) param), bAmount);
            usleep(SLEEP_TIME * arrival);
          }
          printf("closing file %s\n", filename);
          
          done++;
          fclose(thread_file);
          pthread_exit(NULL);
          return;
        }
	//Generate initial wait time
	int initialWait = expo(((struct tInfo *) param)->avgA);
	while (initialWait < ((struct tInfo *) param)->minA)
		initialWait = expo(((struct tInfo *) param)->avgA);
	int counter = 0;
	int bCounter = 0;
	printf("My initial wait is: %d\n", initialWait);
	usleep(SLEEP_TIME * initialWait);
	
	bAmount = expo(((struct tInfo *) param)->avgB);
	while (bAmount < ((struct tInfo *) param)->minB)
		bAmount = expo(((struct tInfo *) param)->avgB);
	
	add(((struct tInfo *) param), bAmount);
	
	printf("Burst amount: %d added to queue by id: %d by waiting: %d\n", bAmount, ((struct tInfo *) param)->index, initialWait);
        if(fileoutput) {
          char filename[200];
          sprintf(filename, "%s-%d.txt", inprefix, ((struct tInfo *) param)->index);
          FILE* thread_file = fopen(filename, "w");
          fprintf(thread_file, "%d %d\n", initialWait, bAmount);
          fclose(thread_file);
        }
	bCounter++;
	counter = 0;
	
	//Generating the rest of the arrival times
	
	while (bCounter < ((struct tInfo *) param)->bCount){
		int bAmount = expo(((struct tInfo *) param)->avgB);
		while (bAmount < ((struct tInfo *) param)->minB)
			bAmount = expo(((struct tInfo *) param)->avgB);
		int arrival = expo(((struct tInfo *) param)->avgA);
		while (arrival < ((struct tInfo *) param)->minA)
			arrival = expo(((struct tInfo *) param)->avgA);
		//printf("bcounter is : %d\n", bCounter);
		
		usleep(SLEEP_TIME * arrival);	
		add(((struct tInfo *) param), bAmount);
                if(fileoutput) {
                  char filename[200];
                  sprintf(filename, "%s-%d.txt", inprefix, ((struct tInfo *) param)->index);
                  FILE* thread_file = fopen(filename, "a");
                  fprintf(thread_file, "%d %d\n", arrival, bAmount);
                  fclose(thread_file);
                }

		printf("Burst amount: %d added to queue by id: %d by waiting: %d\n", bAmount, ((struct tInfo *) param)->index, arrival);
		//printf("id %d bursted\n", ((struct tInfo *) param)->index);
		bCounter++;
	}
	
	pthread_exit(NULL);
}

void add(struct tInfo *burst, int bAmount){
	struct rq *temp, *current;
        if(bAmount == 0) return;
	current = (rq *)malloc((sizeof(rq)));
	current-> threadIndex = burst -> index;
	current-> length = bAmount;
	
	gettimeofday(&current_time, NULL);
	current-> timeCreated = (current_time.tv_usec / 1000);
	current->next = NULL;
	
	pthread_mutex_lock(&lock);
	current-> burstIndex = bIndex;
	if (head == NULL){
		head = temp = current;

		bIndex++;
	} else {
		temp = head;
		while(temp->next != NULL) temp = temp->next;
		temp->next = current;
		bIndex++;
	}
	
	pthread_mutex_unlock(&lock);
	pthread_cond_signal(&bGenerated);
	
}

void print_queue(int line)
{
  struct rq *temp_;	
  pthread_mutex_lock(&lock);
  temp_ = head;	
  printf("=====================\n");
  while(temp_ != NULL){

    printf("Line: %d Thread Index: %d , burstIndex: %d, length: %d, timecreated: %d\n", line, temp_->threadIndex, temp_->burstIndex,temp_->length,temp_->timeCreated);
    temp_ = temp_->next;
  }
  pthread_mutex_unlock(&lock);
  printf("=====================\n");
}

void fcfs(){
	int bCounter = 0;
	//pthread_cond_wait (&cond3,&mux);
	struct rq *temp_;	
	while (1){


		print_queue(__LINE__);
		pthread_mutex_lock(&lock);
		while(head == NULL) {
			pthread_cond_wait(&bGenerated, &lock);	
		}
		temp_ = head;
		head = head->next;


		if( temp_ != NULL){
			usleep(temp_->length * SLEEP_TIME);
			// free the previous head
			free(temp_);
		}
		pthread_mutex_unlock(&lock);
		print_queue(__LINE__);
		bCounter++;
                if(fileinput ==1) {
			if (done >= n && head == NULL) {
				break;
			} 
                }
		if(fileinput ==0) {
		if ((bIndex >= n * bCount) && ( head == NULL)) break;
                }
	}
}
void sjf(struct tInfo *burst){
  int bCounter = 0;
  	
	struct rq *temp_, *temp_1, *prev;	
	while (1){
		print_queue(__LINE__);
                pthread_mutex_lock(&lock);
		while(head == NULL) {
			pthread_cond_wait(&bGenerated, &lock);	
		}
		
		if(head->next == NULL){
		
		temp_ = head;
		head = head->next;


		if( temp_ != NULL){
			usleep(temp_->length * SLEEP_TIME);
			// free the previous head
			free(temp_);
		}
                pthread_mutex_unlock(&lock);
		
		
		} else {
			
			//find the shortest burstIndex
			
			
			temp_ = head;
			temp_1 = head;	
			int location = temp_->burstIndex;	
			while ( temp_1 != NULL){
				if (temp_->length > temp_1->length && temp_1->length > 0){
					location = temp_1->burstIndex;
					temp_ = temp_1;
					temp_1 = temp_1->next;
					
				} else {
					temp_1 = temp_1->next;
					
				}	
			}
			prev = NULL;
			temp_ = head;
			temp_1 = head;
			
			
			while(temp_1!= NULL && temp_->burstIndex != location){
				
				prev = temp_;
				temp_ = temp_1;
				temp_1 = temp_1->next;	
			}
			
			if(prev != NULL){
				prev->next = temp_->next;
			} else {
				head = head->next;
			}
			if( temp_ != NULL){
				usleep(temp_->length * SLEEP_TIME);                     
				// free the previous head
				free(temp_);
			}
		        pthread_mutex_unlock(&lock);
		}
		print_queue(__LINE__);
		bCounter++;

                if(fileinput ==1) {
			if (done >= n && head == NULL) {

				break;
			} 
                }
                if(fileinput ==0) {
		if ((bIndex >= n * bCount) && ( head == NULL)) break;
                }

	}
}

void prio(struct tInfo *burst){
  int bCounter = 0;
  	
	struct rq *temp_, *temp_1, *prev;	
	while (1){

		print_queue(__LINE__);
                pthread_mutex_lock(&lock);

		while(head == NULL) {
			pthread_cond_wait(&bGenerated, &lock);	
		}
		
		if(head->next == NULL){
		
		temp_ = head;
		head = head->next;


		if( temp_ != NULL){
			usleep(temp_->length * SLEEP_TIME);
			// free the previous head
			free(temp_);
		}
                pthread_mutex_unlock(&lock);
		
		
		} else {
			
			//find the shortest thread Index
			
			
			temp_ = head;
			temp_1 = head;	
			int location = temp_->threadIndex;	
			while ( temp_1 != NULL){
				if (temp_->threadIndex > temp_1->threadIndex && temp_1->threadIndex > 0){
					location = temp_1->threadIndex;
					temp_ = temp_1;
					temp_1 = temp_1->next;
					
				} else {
					temp_1 = temp_1->next;
					
				}	
			}
			

			prev = NULL;
			temp_ = head;
			temp_1 = head;
			

			while(temp_1!= NULL && temp_->threadIndex != location){
				
				
				prev = temp_;
				temp_ = temp_1;
				temp_1 = temp_1->next;	
			}
			

			if(prev != NULL){
				prev->next = temp_->next;
			} else {
				head = head->next;
			}
			if( temp_ != NULL){
				usleep(temp_->length * SLEEP_TIME);                     
				// free the previous head
				free(temp_);
			}
		        pthread_mutex_unlock(&lock);
		}
		print_queue(__LINE__);
		bCounter++;
                if(fileinput ==1) {
			if (done >= n && head == NULL) {
				break;
			} 
                }
                if(fileinput ==0) {
		if ((bIndex >= n * bCount) && ( head == NULL)) break;
                }
	}
}

void vruntime(struct tInfo *threads){
  int bCounter = 0;
  	
	struct rq *temp_, *temp_1, *prev;	
	int tIndex;
	int lowest;
	while (1){
		
		printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		print_queue(__LINE__);
		pthread_mutex_lock(&lock);
		while(head == NULL) {
			if(fileinput ==1) {
				if (done == 1) break;
			}
			pthread_cond_wait(&bGenerated, &lock);	
		}
		
		if(head->next == NULL){
		
		temp_ = head;
		int tIndex = head->threadIndex;
		int lowest = threads[tIndex - 1].runtime;

		head = head->next;


		if( temp_ != NULL){
 
			usleep(temp_->length * SLEEP_TIME);
			threads[tIndex - 1].runtime =  threads[tIndex-1].runtime + ((temp_->length)*(0.7+(0.3*temp_->threadIndex)));  
			// free the previous head
			free(temp_);

		}
                pthread_mutex_unlock(&lock);
		
		
		} else {
			
			//find the shortest thread Index
			tIndex = head->threadIndex;
			lowest = threads[tIndex - 1].runtime;
			
			printf("============================================================================\n");
			for(int i = 0; i < n; i++){
				printf("THREAD ID: %d, runtime value: %d\n",threads[i].index,threads[i].runtime);
			}
			printf("============================================================================\n");
			temp_ = head;
			while(temp_!= NULL){
				if (threads[temp_->threadIndex].runtime < lowest){
					lowest = threads[temp_->threadIndex].runtime;
					tIndex = temp_->threadIndex;
					
				}
				temp_ = temp_->next;
			}
			
			
			prev = NULL;
			temp_ = head;
			temp_1 = head;
	
			while(temp_1!= NULL && temp_->threadIndex != tIndex){
				
				
				prev = temp_;
				temp_ = temp_1;
				temp_1 = temp_1->next;	
			}
			
			if(prev != NULL){
				prev->next = temp_->next;
			} else {
				head = head->next;
			}
			if( temp_ != NULL){
				usleep(temp_->length * SLEEP_TIME);
				threads[tIndex - 1].runtime =  threads[tIndex-1].runtime + ((temp_->length)*(0.7+(0.3*temp_->threadIndex)));                    
				// free the previous head
				free(temp_);
			}
		        pthread_mutex_unlock(&lock);
		}
		print_queue(__LINE__);
		bCounter++;
		if(fileinput ==1) {
			if (done >= n && head == NULL) {
				break;
			} 
                }
		if(fileinput ==0) {
			if ((bIndex >= n * bCount) && ( head == NULL)) break;
                }
	}
}


int main(int argc, char *argv[])
{
	done = 0;
	head = NULL;
	pthread_mutex_init (&lock, NULL); 
	pthread_cond_init(&bGenerated, NULL);

	srand( (unsigned) time(NULL));
	//Checking arguments
	
	if (argc != 8 && argc != 5 && argc != 9){
		printf("Invalid number of arguments, correct use is:\n ./schedule 3 75 100 200 1000 1500 FCFS or \n ./schedule 5 FCFS -f infile\n");
		printf("%d", argc);
		return -1;
	}
	clock_t begin = clock();
	if (argc == 5){
		n = atoi(argv[1]);
		char *alg = argv[2];
		char *filename = argv[4];
		pthread_t w[n];
	
		fileinput = 1;
		strncpy(inprefix, argv[4],50);
		inprefix[49] = '\0';
		printf("using input file: %s for %d thread with %s alg\n", inprefix,n, alg);
		pthread_t s;
		
		

		
		struct tInfo thread_info[n];
		int ret;
		
		for(int i = 0; i < n; i++){
			
			
                       thread_info[i].index = i+1;
                       thread_info[i].runtime = 0;

			pthread_create(&(w[i]), NULL, generateBurst, (void*) &(thread_info[i]));
		
		}
		
		if(strcmp(alg, "FCFS") == 0){
			pthread_create(&s, NULL, fcfs,(void*)&(thread_info));
		} else if (strcmp(alg, "SJF") == 0){
			pthread_create(&s, NULL, sjf,(void*)&(thread_info));
		} else if (strcmp(alg, "PRIO") == 0){
			pthread_create(&s, NULL, prio,(void*)&(thread_info));
		} else if (strcmp(alg, "VRUNTIME") == 0){
			pthread_create(&s, NULL, vruntime,(void*)&(thread_info));
		} else {
			printf("Invalid algorithm choice\nCorrect options are: FCFS SJF PRIO VRUNTIME\n");
			printf("Your algorithm choice is: %s \n", alg);
			return -1;
		}
		
                for (int i = 0; i < n; i++){
                  ret = pthread_join(w[i], NULL);
                }
                pthread_join(s, NULL);

                return;
	} else {
	// Converting char to int
	char *alg = argv[7];
	int avgA = atoi(argv[6]);
	int minA = atoi(argv[5]);
	int avgB = atoi(argv[4]);
	int minB = atoi(argv[3]);
	bCount = atoi(argv[2]);
	n = atoi(argv[1]);
        if(argc == 9) {
          fileoutput = 1;
          strncpy(inprefix, argv[8],50);
          inprefix[49] = '\0';
        }

	
	pthread_t w[n];
	
	pthread_t s;
	
	
	struct tInfo thread_info[n];
	int ret;
	
	for(int i = 0; i < n; i++){
		thread_info[i].index = i+1;
		thread_info[i].avgA = avgA;
		thread_info[i].avgB = minA;
		thread_info[i].minA = avgB;
		thread_info[i].minB = minB;
		thread_info[i].bCount = bCount;
		thread_info[i].runtime = 0;

		pthread_create(&(w[i]), NULL, generateBurst, (void*) &(thread_info[i]));
		
	}
	if(strcmp(alg, "FCFS") == 0){
		pthread_create(&s, NULL, fcfs,(void*)&(thread_info));
	} else if (strcmp(alg, "SJF") == 0){
		pthread_create(&s, NULL, sjf,(void*)&(thread_info));
	} else if (strcmp(alg, "PRIO") == 0){
		pthread_create(&s, NULL, prio,(void*)&(thread_info));
	} else if (strcmp(alg, "VRUNTIME") == 0){
		pthread_create(&s, NULL, vruntime,(void*)&(thread_info));
	} else {
		printf("Invalid algorithm choice\nCorrect options are: FCFS SJF PRIO VRUNTIME\n");
		printf("Your algorithm choice is: %s \n", alg);
		return -1;
	}
 	
	for (int i = 0; i < n; i++){
		ret = pthread_join(w[i], NULL);
	}

	ret = pthread_join(s, NULL);
	
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	
	printf("The execution took: %f seconds\n", time_spent*1000);
	
	//showing that the queue is empty at the end
        struct rq *temp;	
	temp = head;
	while(temp != NULL){
		printf("Thread Index: %d , burstIndex: %d, length: %d, timecreated: %d\n", temp->threadIndex, temp->burstIndex,temp->length,temp->timeCreated);
		temp = temp->next;
	}
	
	pthread_mutex_destroy(&lock);

	return 1;
	}
}

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#define GPU_POWER_PATH "/sys/bus/iio/devices/iio:device0/in_power0_input"
#define CPU_POWER_PATH "/sys/bus/iio/devices/iio:device0/in_power1_input"
#define SOC_POWER_PATH "/sys/bus/iio/devices/iio:device0/in_power2_input"

#define CV_POWER_PATH "/sys/bus/iio/devices/iio:device1/in_power0_input"
#define VDDRQ_POWER_PATH "/sys/bus/iio/devices/iio:device1/in_power1_input"
#define SYS5V_POWER_PATH "/sys/bus/iio/devices/iio:device1/in_power2_input"

#define GPU_TEMP_PATH "/sys/devices/virtual/thermal/thermal_zone1/temp"
#define CPU_TEMP_PATH "/sys/devices/virtual/thermal/thermal_zone0/temp"

#define TRACE_PATH "./data/"
#define TRACE_FILE "power_consumption_two-stage_delopt55"
#define TRACE_FORM ".csv"

#define ITER 1000
#define ITER_S 0
#define REC_NUM 3
#define PATH_NUM 6
#define MEM_INFO "/proc/meminfo"

static unsigned int global_count = 0;
static int data_arr[PATH_NUM][30] = {0,};
static int totMEM = 0;
static int mCPU, mGPU;
static double temp_arr[2][ITER] = {0,};
double rt_arr[3] = {0,};
static int detail[REC_NUM*ITER] = {0,};


double gettimeafterboot()
{
	struct timespec time_after_boot;
	clock_gettime(CLOCK_MONOTONIC,&time_after_boot);
	return (time_after_boot.tv_sec*1000+time_after_boot.tv_nsec*0.000001);
}


void *read_memory()
{
    char cmd[1024];
    int nMemTot =0;
    int nMemAva =0;
    int nMapMemUsed =0;

//  sprintf(cmd, "/proc/%d/status", getpid());
    sprintf(cmd, "/proc/meminfo");
    FILE* fp = fopen(MEM_INFO, "r");
    if(fp == NULL) return;

    while(fgets(cmd, 1024, fp) != NULL) {
        if(strstr(cmd, "MemTotal"))  {
            char t[32];
            char size[32];
            sscanf(cmd, "%s%s", t, size);
            nMemTot = atoi(size);
        }
        else if(strstr(cmd, "MemAvailable")) {
            char t[32];
            char size[32];
            sscanf(cmd, "%s%s", t, size);
            nMemAva = atoi(size);
        }
        else if(strstr(cmd, "NvMapMemUsed")) {
            char t[32];
            char size[32];
            sscanf(cmd, "%s%s", t, size);
            nMapMemUsed = atoi(size);
            break;
        }
    }
    fclose(fp);

    mGPU = nMapMemUsed/1024;
    mCPU = ((nMemTot-nMemAva) - nMapMemUsed)/1024;
    totMEM = mCPU + mGPU;

//    data_mem = (nMemTot-nMemAva)/1024;
//    printf("+ nMemTot: %dkB, nMemAva: %dkB\n",nMemTot, nMemAva);
//    printf("+ nMemSub: %dkB (=%dMB)\n",nMemTot-nMemAva,(nMemTot-nMemAva)/1024);
//    printf("+ nMemSub: %dMB\n",data_mem);
    printf("Total memory Usage %dMB\nmCPU : %dMB, mGPU : %dMB\n",totMEM, mCPU, mGPU);
}


// read_data
void *read_data(void *ptr)
{
	double st_time,cur_time;
//	st_time = gettimeafterboot();
	FILE *fp;
	char path[8][150] = {GPU_POWER_PATH, CPU_POWER_PATH, 
						 SOC_POWER_PATH, CV_POWER_PATH, 
						 VDDRQ_POWER_PATH, SYS5V_POWER_PATH,
						 GPU_TEMP_PATH,CPU_TEMP_PATH};
	char n_path[8][20] ={"GPU_POWER", "CPU_POWER", 
						 "SOC_POWER", "CV_POWER", 
						 "VDDRQ_POWER", "SYS5V_POWER",
						 "GPU_TEMP","CPU_TEMP"};
	int each_data=0;

////////read and write 
	for(int paths=0; paths<PATH_NUM; paths++){
		fp = fopen(path[paths], "r");
		if(fp == NULL) printf("File open fail.");
		fscanf(fp,"%d", &each_data);
//		printf("%s:%d\n",n_path[paths],each_data);
		data_arr[paths][0] = each_data;
		fclose(fp);
	}
}


void WriteData()
{
	int read_count;
	char t_file_path[100] = "";
	FILE *t_fp_t;
	FILE *t_fp_p;

	strcat(t_file_path, TRACE_PATH);
	strcat(t_file_path, TRACE_FILE);
	strcat(t_file_path, TRACE_FORM);
	printf("file path : %s\n",t_file_path);
	
	t_fp_p = fopen(t_file_path, "w+");
	

	fprintf(t_fp_p,"GPUPower, CPUPower\n");
	// write INFO
	// iteration starts After ITER_Start
	for(int j=0; j<global_count; j++){////
		fprintf(t_fp_p,"%d,%d\n", data_arr[0][j], data_arr[1][j]);
	}
	
	fclose(t_fp_p);
}


void WriteData_linebyline()
{
	int read_count;
	static int flag = 0;
	char t_file_path[100] = "";
	FILE *t_fp_t;
	FILE *t_fp_p;

	strcat(t_file_path, TRACE_PATH);
	strcat(t_file_path, TRACE_FILE);
	strcat(t_file_path, TRACE_FORM);
//	printf("file path : %s\n",t_file_path);
	
	t_fp_p = fopen(t_file_path, "a+");
	
	if(!flag){
//		printf("flag = %d\n",flag);
		fprintf(t_fp_p,"GPUPower,CPUPower,SOCPower,CVPower,VDDRQPower,SYS5VPower\n");//power etc.
//		fprintf(t_fp_p,"GPUPower,CPUPower,rt\n");//power etc.
//		fprintf(t_fp_p,"totMEM,mCPU,mGPU\n");//memory
		flag = 1;
	}

	// write INFO
	// iteration starts After ITER_Start
    for(int i=0; i<PATH_NUM-1; i++){
	    fprintf(t_fp_p,"%d,", data_arr[i][0]);//power etc.
    }
	fprintf(t_fp_p,"%d\n", data_arr[PATH_NUM-1][0]);//power etc.
//	fprintf(t_fp_p,"%d,%d,%0.2lf\n", data_arr[0][0], data_arr[1][0], rt_arr[0]);//power etc.
//	fprintf(t_fp_p,"%d,%d,%d\n",totMEM,mCPU,mGPU);//memory
//	printf("%d,%d\n", data_arr[0][global_count], data_arr[1][global_count]);
	
	fclose(t_fp_p);
}

void* time_in_thread()
{
//    printf("time_in_thread()\n");
    usleep(10*1000);
}

volatile sig_atomic_t flag = 0;
void my_function(int sig){ // can be called asynchronously
	flag = 1; // set flag
}

int main(){
	
    pthread_t time_thread;
    // Register signals 
	signal(SIGINT, my_function); 
	//      ^          ^
	//  Which-Signal   |-- which user defined function registered
	while(1){
        pthread_create(&time_thread,0,time_in_thread,0);
//		printf("global_count : %d\n",global_count);
		read_data(0);
//        read_memory();
		if(flag){ // my action when signal set it 1
			printf("\n !!\n");
			flag = 0;
			break;
		}
		WriteData_linebyline();
		global_count++;
        pthread_join(time_thread,0);
	}
//	WriteData();
  
  return 0;
}  


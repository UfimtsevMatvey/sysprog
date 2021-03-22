#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ucontext.h>
#include <signal.h>
#include <time.h>

static ucontext_t uctx_main, uctx_func1, uctx_func2;
static ucontext_t* uctx_func;

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define stack_size 1024 * 1024 

void merge_sort(int*, int, int);
void merge(int*, int, int, int);
int merge_files(int**, int*, int);
void test_merge();
void quicksort(int*, int, int);
void quicksort_withoutreq(int*, int, int);
int split_array(int*, int, int);//return length first part array
static void* allocate_stack()
{
	void *stack = malloc(stack_size);
	stack_t ss;
	ss.ss_sp = stack;
	ss.ss_size = stack_size;
	ss.ss_flags = 0;
	sigaltstack(&ss, NULL);
	return stack;
}
static void my_coroutine(int argc, int nargv, const char* argv[], int* buff, int* buff_size, long* time)
{
    struct timeval begin, end, t1, t2, t3, t4;
    gettimeofday(&begin,NULL);
    int i = 0;
    long microseconds = 0;
    long seconds = 0;
    int n_context;
    int c_context = nargv;
    int* nbuff;
    n_context = (nargv >= argc - 2) ? 0 : nargv + 1;
    gettimeofday(&end,NULL);
    seconds = end.tv_sec - begin.tv_sec;
    microseconds = end.tv_usec - begin.tv_usec + seconds*1e6;
    if (swapcontext(&uctx_func[c_context], &uctx_func[n_context]) == -1)
	    handle_error("swapcontext");
    gettimeofday(&begin,NULL);
    FILE* fd = fopen(argv[nargv + 1], "r");

    gettimeofday(&end,NULL);
    seconds = end.tv_sec - begin.tv_sec + seconds; 
    microseconds = end.tv_usec - begin.tv_usec + seconds*1e6 + microseconds;

    
    if (swapcontext(&uctx_func[c_context], &uctx_func[n_context]) == -1)
	    handle_error("swapcontext");
    gettimeofday(&begin,NULL);
    while(fscanf(fd, "%d", &buff[i]) == 1) i = i + 1;
    gettimeofday(&end,NULL);
    seconds = end.tv_sec - begin.tv_sec + seconds; 
    microseconds = end.tv_usec - begin.tv_usec + seconds*1e6 + microseconds;   
    if (swapcontext(&uctx_func[c_context], &uctx_func[n_context]) == -1)
	    handle_error("swapcontext");
    gettimeofday(&begin,NULL);
    *buff_size = i;
    gettimeofday(&end,NULL);
    seconds = end.tv_sec - begin.tv_sec + seconds; 
    microseconds = end.tv_usec - begin.tv_usec + seconds*1e6 + microseconds;   
    if (swapcontext(&uctx_func[c_context], &uctx_func[n_context]) == -1)
        handle_error("swapcontext");
    gettimeofday(&begin,NULL);
    quicksort(buff, 0, i - 1);
    gettimeofday(&end,NULL);
    seconds = end.tv_sec - begin.tv_sec + seconds; 
    microseconds = end.tv_usec - begin.tv_usec + seconds*1e6 + microseconds;   
    *time = microseconds;
    printf("Time measured: %.3ld microseconds.\n", microseconds);
    if (swapcontext(&uctx_func[c_context], &uctx_func[n_context]) == -1)
	    handle_error("swapcontext");


}

int main(int argc, const char *argv[])
{
    struct timespec ts1, ts2;
    clock_gettime(CLOCK_REALTIME, &ts1);
    const char* res_file_name = "res.txt";
    char** stacks;

    struct stat st;
    int size;
    int i = 0;
    FILE* fdr;
    FILE** fd;
    int** buff;
    int* sizes;
    long* time;
    long wtime = 0;
    if(argc >= 2){
        stacks = (char**)malloc((argc - 1)*sizeof(char*));
        time = (long*)malloc((argc - 1)*sizeof(long));
        fd = (FILE**)malloc((argc - 1)*sizeof(FILE*));
        buff = (int**)malloc((argc - 1)*sizeof(int*));
        sizes = (int*)malloc((argc - 1)*sizeof(int));
        fdr = fopen(res_file_name, "w");
        uctx_func = (ucontext_t*)malloc((argc - 1) * sizeof(ucontext_t));

        for(int j = 0; j < argc - 1; j++){
            i = 0;
            /*create stack for each coroutine*/
            stacks[j] = allocate_stack();
	        if (getcontext(&uctx_func[j]) == -1)
		        handle_error("getcontext");

            uctx_func[j].uc_stack.ss_sp = stacks[j];
	        uctx_func[j].uc_stack.ss_size = stack_size;
	        uctx_func[j].uc_link = &uctx_main;

            stat(argv[j + 1], &st);
            sizes[j] = st.st_size;
            //printf("size file = %d \n", sizes[j]);
            buff[j] = (int*)malloc(st.st_size);

	        makecontext(&uctx_func[j], my_coroutine, 6, argc, j, argv, buff[j], &sizes[j], &time[j]);
        }
        //printf("main: swapcontext(&uctx_main, &uctx_func)\n");
	    if (swapcontext(&uctx_main, &uctx_func[0]) == -1)
		    handle_error("swapcontext");
        //printf("returned to main\n");
        /*for(i = 0; i < argc - 1; i++)
            printf("size = %d \n", sizes[i]);*/
        
        size = merge_files(buff, sizes, argc - 1);
        //size = 10000;
        //printf("size = %d \n", size);
        for(int i = 0; i < argc - 1; i++)
            wtime += time[i];
        printf("stime coroutime = %ld\n", wtime);
        for(i = 0; i < size; i++){
            //printf("%d, buff = %d \n", i, buff[0][i]);
            fprintf(fdr,"%d ", buff[0][i]);
        }
        //printf("end\n");


        for(int j = 0; j < argc - 1; j++) free(buff[j]);
        free(buff);
        fclose(fdr);
        free(fd);
        free(sizes);
    }
    clock_gettime(CLOCK_REALTIME, &ts2);
    if (ts2.tv_nsec < ts1.tv_nsec) {
        ts2.tv_nsec += 1000000000;
        ts2.tv_sec--;
    }
    printf("time of all work \n");
    printf("%ld.%09ld\n", (long)(ts2.tv_sec - ts1.tv_sec), ts2.tv_nsec - ts1.tv_nsec);
    return 0;
}


void test_merge()
{
    int a[10] = {6,8,32,54,66,79,81,92,333, 543};
    int b[10] = {1,10,12,64,96,129,130,145,250, 501};
    int c[20];
    for(int i = 0; i < 10; i++)
        c[i] = a[i];
    for(int i = 10; i < 20; i++)
        c[i] = b[i - 10];
    merge(c, 0, 20 - 1 , 9);
    for(int i = 0; i < 20; i++)
    printf("%d \n",c[i]);
}
int merge_files(int** buff, int* sizes, int n)
{
    int length = 0;
    int* nbuff;
    for(int i = 0; i < n; i++) length += sizes[i];
    //printf("length = %d \n", length);
    buff[0] = (int*)realloc(buff[0], (length)*sizeof(int));
    if(buff[0] == NULL) printf("FATAL ERROR\n");
    //free(buff[0]);
    //buff[0] = nbuff;
    int s = sizes[0];
    for(int i = 1; i < n; i++){
        for(int j = 0; j < sizes[i]; j++){
            buff[0][s + j] = buff[i][j];
        }
        s += sizes[i];
        merge(buff[0], 0, s - 1, s - sizes[i] - 1);
    }
    return s;
}
void merge_sort(int *array, int low, int high)
{
    int temp = 0;
    if(high - low > 1){
        int ave = (low + high)/2 - 1;
        merge_sort(array, low, ave);
        merge_sort(array, ave + 1, high);
        merge(array, low, high, ave);
    }
    else if(high != low){
        if(array[low] > array[high]){
            temp = array[low];
            array[low] = array[high];
            array[high] = temp;
        } 
    }
}
void merge(int *array, int low, int high, int split_point)
{
    int i = 0;
    int j = 0;
    int left_count = low;
    int right_count = split_point + 1;
    int N = high - low + 1;
    int *temp_array = (int*)malloc(N*sizeof(int));
    for(i = 0; i < N; i++){
        if(right_count <= high && left_count <= split_point){
            if(array[left_count] > array[right_count]){
                temp_array[i] = array[right_count];
                ++right_count;
            }
            else{
                temp_array[i] = array[left_count];
                ++left_count;
            }
        }
        else if(right_count > high){
            temp_array[i] = array[left_count + j];
            j++;
        }
        else if(left_count > split_point){
            temp_array[i] = array[right_count + j];
            j++;
        }
        
    }
    for(i = 0; i < N; i++)
        array[low + i] = temp_array[i];
    free(temp_array);
}

void quicksort(int *Array, int low, int high)
{
    /*printf("low = %d\n", low);
    printf("high = %d\n", high);*/
    int temp = 0;
    int N = high - low;
    int split_point = 0;

    int i = low;
    int j = high;
    int avind = (int)((high + low)/2);
    int p = Array[avind];
    do
    {
        while (Array[i] < p) i = i + 1;
        while (Array[j] > p) j = j - 1;
        if(i <= j){
            temp = Array[i];
            Array[i] = Array[j];
            Array[j] = temp;    
            i++;
            j--;    
        }
    } while (i <= j);
    if(j > low) quicksort(Array, low, j);
    if(i < high) quicksort(Array, i, high);
}
/*void quicksort(int* number, int first,int last){
   int i, j, pivot, temp;

   if(first<last){
      pivot=first;
      i=first;
      j=last;

      while(i<j){
         while(number[i]<=number[pivot]&&i<last)
            i++;
         while(number[j]>number[pivot])
            j--;
         if(i<j){
            temp=number[i];
            number[i]=number[j];
            number[j]=temp;
         }
      }

      temp=number[pivot];
      number[pivot]=number[j];
      number[j]=temp;
      quicksort(number,first,j-1);
      quicksort(number,j+1,last);

   }
}*/

int split_array(int *array, int low, int high)
{
    int pivot = array[(high + low)/2];
    int temp = 0;
    int i = low - 1;
    int j = high + 1;
    while(1){
        do{
            i++;
        } while (array[i] < pivot);
        do{
            j--;
        } while (array[j] > pivot);
        if(i >= j)
            return j;
        temp = array[i];
        array[i] = array[j];
        array[j] = temp;
        i++;
        j--;   
    }
}
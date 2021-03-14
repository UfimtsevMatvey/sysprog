#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/stat.h>

/*
FILE *fopen( const char * filename, const char * mode );
int fscanf(FILE *stream, const char *format, ...)
int fclose( FILE *fp );
*/
void merge_sort(int*, int, int);
void merge(int*, int, int, int);
int merge_files(int**, int*, int);
void test_merge();
void quicksort(int*, int, int);
void quicksort_withoutreq(int*, int, int);
int split_array(int*, int, int);//return length first part array
int main(int argc, const char *argv[])
{
    const char* res_file_name = "res.txt";
    struct stat st;
    int size;
    int i = 0;
    FILE* fdr;
    if(argc >= 2){
        FILE** fd = (FILE**)malloc((argc - 1)*sizeof(FILE*));
        int** buff = (int**)malloc((argc - 1)*sizeof(int*));
        int* sizes = (int*)malloc((argc - 1)*sizeof(int));
        fdr = fopen(res_file_name, "w");
        for(int j = 0; j < argc - 1; j++){
            fd[j] = fopen(argv[j + 1], "r");
            stat(argv[j + 1], &st);
            size = st.st_size;
            buff[j] = (int*)malloc(size);
            i = 0;
            while(fscanf(fd[j], "%d", &buff[j][i]) == 1) i = i + 1;
            size = i;
            sizes[j] = size;
            quicksort(buff[j], 0, size - 1);
        }
        //test_merge();
        size = merge_files(buff, sizes, argc - 1);

        for(int i = 0; i < size; i++){
            printf("%d, buff = %d \n", i, buff[0][i]);
            fprintf(fdr,"%d ", buff[0][i]);
        }

        for(int j = 0; j < argc - 1; j++){
            free(buff[j]);
            fclose(fd[j]);
        }
        free(buff);
        fclose(fdr);
        free(fd);
    }
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
    for(int i = 0; i < n; i++) length += sizes[i];

    buff[0] = (int*)realloc(buff[0], (length - 1)*sizeof(int));
    int s = sizes[0];
    for(int i = 1; i < n; i++){
        for(int j = 0; j < sizes[i]; j++) buff[0][s + j] = buff[i][j];
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
    else
        if(high != low){
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
    int *temp_array = (int*)calloc(N, sizeof(int));
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
    int temp = 0;
    int N = high - low;
    int split_point = 0;

    int i = low;
    int j = high;
    int p = Array[(high + low)/2];
    /*for(int i = 0; i < 10; i++)
        printf("%i\t",Array[i]);
    printf("\n");*/
    do
    {
        while (Array[i] < p) i++;
        while (Array[j] > p) j--;

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
#include <mpi.h>
#include <stdio.h>

int main(int argc, char* argv[]){
   int pid, np;
   MPI_Status status;
   int a[1000];
   int n;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &pid);
   MPI_Comm_size(MPI_COMM_WORLD, &np);
   
   if(pid==0){
     printf("Hello from master process.\n");
     printf("Number of slave processes is %d\n\n\n", np-1);
    
     printf("Please enter size of array...\n");
     scanf("%d", &n);
     printf("Please enter array elements...\n");
     for(int i=0; i<n; i++){
        scanf("%d", &a[i]);
     }
     printf("\n\n");
     int num_of_slaves = np-1;
     int elements_per_process= n / num_of_slaves;
     int remainig_elements= n % num_of_slaves;
     int index=0;
    
     for(int i=1; i<np; i++){
        int count;
        if(i <= remainig_elements){
           count=elements_per_process+1;
        }else{
            count=elements_per_process;
        }

        MPI_Send(&count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(&a[index], count, MPI_INT, i, 1, MPI_COMM_WORLD);
        index+=count;
     }

     int final_max=a[0];
     int final_idx=0;
     index=0;
  
     for(int i=1; i<np; i++){
        int local_max, local_idx;
        MPI_Recv(&local_max, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&local_idx, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);

        printf("Hello from slave#%d Max number in my partition is %d and index >

        if(local_max > final_max){
           final_max=local_max;
           final_idx= index + local_idx;
        }

        int count;
        if(i <= remainig_elements){
           count=elements_per_process+1;
        }else{
            count=elements_per_process;
        }
        index+=count;

     }
        printf("Master process announces the final max which is %d and its inde>
        printf("Thanks for using our program\n");

   }else{
       int count;
       MPI_Recv(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
       int data[count];
       MPI_Recv(data, count, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
     
       int local_max=data[0];
       int local_idx=0;

      for(int i=1; i<count; i++){
        if(data[i] > local_max){
           local_max=data[i];
           local_idx=i;
        }
       }
       MPI_Send(&local_max, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
       MPI_Send(&local_idx, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
   }
   MPI_Finalize();
   return 0;
}


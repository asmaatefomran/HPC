#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 1024

char CaesarCipherEncryption(char ch, int shift, int op){
    if(ch >= 'a' && ch <= 'z'){
        if(op == 1){
            return (ch - 'a' + shift) % 26 + 'a';
        }
        return (ch - 'a' - shift + 26) % 26 + 'a';
    }
    else if(ch >= 'A' && ch <= 'Z'){
        if(op == 1){
            return (ch - 'A' + shift) % 26 + 'A';
        }
        return (ch - 'A' - shift + 26) % 26 + 'A';
    }
    return ch;
}

int chunk_length(int total_len, int rank, int size) {
    int base = total_len / size;
    int remainder = total_len % size;
    return (rank < remainder) ? base + 1 : base;
}

int main(int argc, char **argv){
    int rank, size;
    char full_input[MAX_LEN], final_result[MAX_LEN], file_name[MAX_LEN];
    int shift = 3;
    int op;
    int totalLength = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == 0){
        printf("Choose input method 1 - Console, 2 - File : ");
        int input_method;
        scanf("%d", &input_method);
        getchar(); 

        if(input_method == 1){
            printf("Enter the text: ");
            fgets(full_input, MAX_LEN, stdin);
            full_input[strcspn(full_input, "\n")] = '\0'; // remove newline
        }
        else{
            FILE *file;
            printf("Enter the file name: ");
            scanf("%s", file_name);
            file = fopen(file_name, "r");
            if(!file){
                printf("Failed to open the file!\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            int readF = fread(full_input, 1, MAX_LEN, file);
            full_input[readF] = '\0';
            fclose(file);
        }

        printf("Choose operation 1 - Encode, 2 - Decode : ");
        scanf("%d", &op);

        totalLength = strlen(full_input);

        for(int i = 1; i < size; i++){
            MPI_Send(&totalLength, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&op, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Recv(&totalLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&op, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int local_len = chunk_length(totalLength, rank, size);
    char* local_input = (char*)malloc((local_len + 1) * sizeof(char));
    local_input[local_len] = '\0';

    if(rank == 0){
        int startInd = 0;
        for(int i = 0; i < size; i++){
            int chunkL = chunk_length(totalLength, i, size);
            if(i == 0){
                memcpy(local_input, full_input+startInd, chunkL);
            }
            else{
                MPI_Send(full_input + startInd, chunkL, MPI_CHAR, i, 2, MPI_COMM_WORLD);
            }
            startInd += chunkL;
        }
    }
    else{
        MPI_Recv(local_input, local_len, MPI_CHAR, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for(int i = 0; i < local_len; i++){
        local_input[i] = CaesarCipherEncryption(local_input[i], shift, op);
    }

    if(rank == 0){
        int startInd = 0;
        for(int i = 0; i < size; i++){
            int chunkL = chunk_length(totalLength, i, size);
            if(i == 0){
                memcpy(final_result+startInd, local_input, chunkL);
            }
            else{
                MPI_Recv(final_result + startInd, chunkL, MPI_CHAR, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            startInd += chunkL;
        }
        final_result[totalLength] = '\0';
        printf("Final Result: %s\n", final_result);
    }
    else{
        MPI_Send(local_input, local_len, MPI_CHAR, 0, 3, MPI_COMM_WORLD);
    }

    free(local_input);
    MPI_Finalize();
    return 0;
}

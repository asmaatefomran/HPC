#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

#define MAX_LEN 1024

void text_work(char* s, int len, int op) {
    int sh = 3;
    for(int i = 0; i < len; ++i) {
        if(s[i] >= 'a' && s[i] <= 'z') {
            int nw = s[i] - 'a';
            if(op == 1) {
                nw += sh;
                nw %= 26;
            } else {
                nw -= sh;
                nw += 26;
                nw %= 26;
            }
            s[i] = 'a' + nw;    
        }
        else if(s[i] >= 'A' && s[i] <= 'Z') {
            int nw = s[i] - 'A';
            if(op == 1) {
                nw += sh;
                nw %= 26;
            } else {
                nw -= sh;
                nw += 26;
                nw %= 26;
            }
            s[i] = 'A' + nw;
        }    
    }
}

int main(int argc, char* argv[]) {
    int rank, size;
    char full_input[MAX_LEN], final_result[MAX_LEN], file_name[MAX_LEN];
    int shift = 3;
    int op;
    int totalLength = 0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    
    if(rank == 0) {
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

        
        char* text = (char*)malloc((totalLength+1)*sizeof(char));
        strcpy(text, full_input);

        int otherP = size - 1;
        int part = totalLength / otherP;
        int rem = totalLength % otherP;
        
        for(int i = 1; i <= otherP; ++i) {
            MPI_Send(&op, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int start = 0;
        for(int i = 1; i <= otherP; ++i) {
            int currpart = part + (i <= rem ? 1 : 0);
            MPI_Send(&currpart, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(&text[start], currpart, MPI_CHAR, i, 2, MPI_COMM_WORLD);
            start += currpart;
        }
        
        start = 0;
        for(int i = 1; i <= otherP; ++i) {
            int currpart;
            MPI_Recv(&currpart, 1, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&text[start], currpart, MPI_CHAR, i, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            start += currpart;
        }
        printf("Result: %s\n", text);
        free(text);
    } else {
        int currlen;
        MPI_Recv(&op, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&currlen, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char* currtext = (char*)malloc((currlen+1)*sizeof(char));
        MPI_Recv(currtext, currlen, MPI_CHAR, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        currtext[currlen] = '\0';
      
        text_work(currtext, currlen, op);
        MPI_Send(&currlen, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
        MPI_Send(currtext, currlen, MPI_CHAR, 0, 4, MPI_COMM_WORLD);
        free(currtext);        
    }

    MPI_Finalize();
    return 0;
}

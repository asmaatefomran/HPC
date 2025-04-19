#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

void text_work(char* s, int len, int op) {
    int sh = 3;
    for(int i = 0; i < len; ++i) {
        if(s[i] >= 'a' && s[i] <= 'z') {
            int nw = s[i] - 'a';
            if(op == 0) {
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
            if(op == 0) {
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
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) setbuf(stdout, NULL);  

    if(rank == 0) {
        char input[1024];
        int input_s, mode;
        
        printf("Enter the type of operation you want (0-->encrypt | 1-->decrypt): ");
        fflush(stdout);  
        scanf("%d", &mode);
        getchar(); 
        
        printf("Enter the text: ");
        fflush(stdout);  
        fgets(input, 1024, stdin);
        input_s = strlen(input);

        if(input_s > 0 && input[input_s-1] == '\n') {
            input[input_s-1] = '\0';
            input_s--;
        }
        
        char* text = (char*)malloc((input_s+1)*sizeof(char));
        strcpy(text, input);

        int otherP = size - 1;
        int part = input_s / otherP;
        int rem = input_s % otherP;
        
        for(int i = 1; i <= otherP; ++i) {
            MPI_Send(&mode, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
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
        int mode, currlen;
        MPI_Recv(&mode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&currlen, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char* currtext = (char*)malloc((currlen+1)*sizeof(char));
        MPI_Recv(currtext, currlen, MPI_CHAR, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        currtext[currlen] = '\0';
      
        text_work(currtext, currlen, mode);
        MPI_Send(&currlen, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
        MPI_Send(currtext, currlen, MPI_CHAR, 0, 4, MPI_COMM_WORLD);
        free(currtext);        
    }

    MPI_Finalize();
    return 0;
}

//Register my signal handler:
//Define a handler function with the signature: void function_name (int arg)
//Declare a struct sigaction variable in your main() function
//Assign the sa_handler field of your struct sigaction variable your signal handling function
//Assign the sa_flags field of your struct sigaction variable SA_RESTART
//Register your signal handler by calling the sigaction() function in your main() function, with SIGINT as the signum argument and a pointer to your struct sigaction for the act argument.
//
/******************************************************************************
*
* dense_mm.c
*
* This program implements a dense matrix multiply and can be used as a
* hypothetical workload.
*
* Usage: This program takes a single input describing the size of the matrices
*        to multiply. For an input of size N, it computes A*B = C where each
*        of A, B, and C are matrices of size N*N. Matrices A and B are filled
*        with random values.
*
* Written Sept 6, 2015 by David Ferry
******************************************************************************/

#include <stdio.h>  //For printf()
#include <stdlib.h> //for exit() and atoi()
#include <signal.h>

const int num_expected_args = 2;
const unsigned sqrt_of_UINT32_MAX = 65536;

//Define a handler function with the signature: void function_name (int arg)
static int i = 0;
//step 5:
char* message = "handler_called\n";
void sigint_handler(int signum){
    i++;
    write(0,message,15);
    return;
}



int main( int argc, char* argv[] ){

    //Declare a struct sigaction variable in your main() function
    struct sigaction ss;

    unsigned index, row, col; //loop indicies
    unsigned matrix_size, squared_size;
    double *A, *B, *C;

    if( argc != num_expected_args ){
        printf("Usage: ./dense_mm <size of matrices>\n");
        exit(-1);
    }

    matrix_size = atoi(argv[1]);

    if( matrix_size > sqrt_of_UINT32_MAX ){
        printf("ERROR: Matrix size must be between zero and 65536!\n");
        exit(-1);
    }

    //Assign the sa_handler field of your struct sigaction variable your signal handling function
    ss.sa_handler = sigint_handler;
    //Assign the sa_flags field of your struct sigaction variable SA_RESTART
    /*
     * This flag affects the behavior of interruptible functions;
     * that is, those specified to fail with errno set to [EINTR]. If set, and a function specified as interruptible is interrupted by this signal, the function shall restart and shall not fail with [EINTR] unless otherwise specified.
     * If an interruptible function which uses a timeout is restarted, the duration of the timeout following the restart is set to an unspecified value that does not exceed the original timeout value.
     * If the flag is not set, interruptible functions interrupted by this signal shall fail with errno set to [EINTR].
     */
    ss.sa_flags = SA_RESTART;
    //Register your signal handler by calling the sigaction() function in your main() function, with SIGINT as the signum argument and a pointer to your struct sigaction for the act argument.
    sigaction(SIGINT, &ss, NULL);

    squared_size = matrix_size * matrix_size;

    printf("Generating matrices...\n");

    A = (double*) malloc( sizeof(double) * squared_size );
    B = (double*) malloc( sizeof(double) * squared_size );
    C = (double*) malloc( sizeof(double) * squared_size );

    for( index = 0; index < squared_size; index++ ){
        A[index] = (double) rand();
        B[index] = (double) rand();
        C[index] = 0.0;
    }

    printf("Multiplying matrices...\n");

    for( row = 0; row < matrix_size; row++ ){
        for( col = 0; col < matrix_size; col++ ){
            for( index = 0; index < matrix_size; index++){
                C[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
            }
        }
    }

    printf("Multiplication done!\n");
    printf("Signal counter:%d\n",i);

    return 0;
}


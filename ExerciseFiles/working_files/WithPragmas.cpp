// Exercises pulled from series of lectures from TAC-HEP located at: https://tac-hep.org/training-modules/uw-gpu-fpga.html

#include <iostream>

// Problem 1 
#define N 16
// Function to perform vector addition
void vector_add(const int *a, const int *b, int *c) {
    #pragma HLS PIPELINE II=1
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
}

/// PIPELINE ensures a new iteration starts every cycle (II=1), increasing throughput.


// Problem 2 
// Function to perform matrix multiplication
void matrix_mult(const int A[N][N], const int B[N][N], int C[N][N]) {
    #pragma HLS ARRAY_PARTITION variable=A complete dim=2
    #pragma HLS ARRAY_PARTITION variable=B complete dim=1
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            #pragma HLS UNROLL factor=4
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

/// ARRAY_PARTITION reduces memory access latency by breaking A and B into smaller parts.
/// UNROLL factor=4 increases parallelism by processing 4 iterations per cycle.

// Problem 3 
// Function to perform dot product
int dot_product(const int *x, const int *y) {
    int result = 0;
    #pragma HLS PIPELINE
    for (int i = 0; i < N; i++) {
        result += x[i] * y[i];
    }
    return result;
}

/// PIPELINE increases throughput by allowing iterations to overlap.

// Problem 4 
// Function to perform element-wise multiplication of two arrays
void elementwise_mult(const int *a, const int *b, int *c) {
    #pragma HLS PIPELINE II=2
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i];
    }
}

/// PIPELINE with II=2 ensures a new iteration starts every 2 cycles, optimizing resource use.


// Problem 5 
// Function to compute the sum of an array
int array_sum(const int *a) {
    int sum = 0;
    #pragma HLS UNROLL factor=2
    for (int i = 0; i < N; i++) {
        sum += a[i];
    }
    return sum;
}

/// UNROLL factor=2 processes 2 iterations per cycle, reducing loop latency.

// Problem 6 
// Function to find the maximum value in an array
int find_max(const int *a) {
    int max_val = a[0];
    #pragma HLS PIPELINE
    for (int i = 1; i < N; i++) {
        if (a[i] > max_val) {
            max_val = a[i];
        }
    }
    return max_val;
}

/// PIPELINE enables overlapping iterations for faster execution.

// Problem 7 
// Function to perform cumulative sum of an array
void cumulative_sum(const int *a, int *b) {
    int sum = 0;
    #pragma HLS PIPELINE
    for (int i = 0; i < N; i++) {
        sum += a[i];
        b[i] = sum;
    }
}

/// PIPELINE ensures that each iteration starts quickly, improving latency.

// Problem 8 
// Function to reverse an array
void reverse_array(const int *a, int *b) {
    #pragma HLS PIPELINE
    for (int i = 0; i < N; i++) {
        b[i] = a[N - 1 - i];
    }
}

/// PIPELINE allows iterations to start earlier, reducing delay.

// Problem 9 

// Function to initialize an array with a specific value
void initialize_array(int *a, int value) {
    #pragma HLS UNROLL
    for (int i = 0; i < N; i++) {
        a[i] = value;
    }
}

/// UNROLL eliminates the loop by generating independent assignments in hardware.

// Problem 10 
// Function to compute the average of an array
float compute_average(const int *a) {
    int sum = 0;
    #pragma HLS PIPELINE
    for (int i = 0; i < N; i++) {
        sum += a[i];
    }
    return static_cast<float>(sum) / N;
}

/// PIPELINE ensures all iterations overlap, speeding up execution.
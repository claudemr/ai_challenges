#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// To debug: fprintf(stderr, "Debug messages...\n");

#define MAX_NUM 15000
#define MAX_T   10000000

enum BigO
{
    O_1,
    O_LOGN,
    O_N,
    O_NLOGN,
    O_N2,
    O_N2LOGN,
    O_N3,
    O_2N,
    NB_O
};

const char *strO[NB_O] = 
    {
        "O(1)",
        "O(log n)",
        "O(n)",
        "O(n log n)",
        "O(n^2)",
        "O(n^2 log n)",
        "O(n^3)",
        "O(2^n)"
    };

struct Sample
{
    double x;  // ]5, 15000[
    double y;  // ]0, 10000000[
};

int main()
{
    int n;  // ]5, 1000[
    struct Sample *samples;
    
    scanf("%d", &n);
    
    samples = malloc(n * sizeof(struct Sample));
    for (int i = 0; i < n; i++)
    {
        int num;
        int t;
        scanf("%d%d", &num, &t);
        // Normalize
        samples[i].x = (double)num / MAX_NUM;
        samples[i].y = (double)t   / MAX_T;
    }
    
    enum BigO bigO = O_1;
    
    printf("%s\n", strO[bigO]);

    return 0;
}

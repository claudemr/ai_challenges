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

// sum(x) / n
double mean(const double *samples, int n)
{
    int i;
    double sum = 0.0;
    for (i = 0; i < n; i++)
        sum += samples[i];

    return sum / n;
}

double sx(const double *samples, int n, double mean)
{
    int i;
    double sum = 0.0;
    for (i = 0; i < n; i++)
    {
        double c = samples[i] - mean;
        sum += c * c;
    }
    return sum;
}

double sxy(const double *x, const double *y, int n, double meanX, double meanY)
{
    int i;
    double sum = 0.0;
    for (i = 0; i < n; i++)
        sum += (x[i] - meanX) * (y[i] - meanY);
    return sum;
}

void derivative(double **dx, double **dy,
                const double *x, const double *y, int n)
{
    int i;
    *dx = malloc((n - 2) * sizeof(double));
    *dy = malloc((n - 2) * sizeof(double));
    for (i = 1; i < n - 1; i++)
    {
        (*dx)[i] = x[i];
        (*dy)[i] = (y[i + 1] - y[i - 1]) / (x[i + 1] - x[i - 1]);
    }
}

void derivative2(double **dx, double **dy,
                 const double *x, const double *y, int n)
{
    int i;
    *dx = malloc((n - 2) * sizeof(double));
    *dy = malloc((n - 2) * sizeof(double));
    for (i = 1; i < n - 1; i++)
    {
        (*dx)[i] = x[i];
        (*dy)[i] = (y[i + 1] - 2 * y[i] + y[i - 1]) / ((x[i + 1] - x[i - 1]) * (x[i + 1] - x[i - 1]));
    }
}

int checkO1(const double *x, const double *y, int n)
{
    double meanX, meanY;
    double linearA, linearB;
    
    #define THRESHOLD 0.0001
    
    meanX = mean(x, n);
    meanY = mean(y, n);
    
    linearB = sxy(x, y, n, meanX, meanY) / sx(x, n, meanX);
    linearA = meanY - linearB * meanX;

    fprintf(stderr, "a=%f b=%f\n", linearA, linearB);
    
    return 0;
}

int main()
{
    int n;  // ]5, 1000[
    double *smpX, *smpY;
    
    scanf("%d", &n);
    fprintf(stderr, "N=%d\n", n);
    
    smpX = malloc(n * sizeof(double));
    smpY = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
    {
        int num;
        int t;
        scanf("%d%d", &num, &t);
    //fprintf(stderr, "%d %d\n", num, t);
        // Normalize
        smpX[i] = (double)num /*/ MAX_NUM*/;
        smpY[i] = (double)t   /*/ MAX_T*/;
        // safety check
        /*if (smpX[i] <= 0.0 || smpX[i] > 1.0 ||
            smpY[i] <= 0.0 || smpY[i] > 1.0)
        {
            fprintf(stderr, "Value out of range: %f %f\n", smpX[i], smpY[i]);
            return -1;
        }*/
    }
    
    double *dY, *ddY;
    double *dX, *ddX;
    checkO1(smpX, smpY, n);
    derivative(&dX, &dY, smpX, smpY, n);
    checkO1(dX, dY, n - 2);
    derivative2(&ddX, &ddY, smpX, smpY, n);
    checkO1(ddX, ddY, n - 2);
    
    enum BigO bigO = O_1;
    
    printf("%s\n", strO[bigO]);

    return 0;
}

/*
Comments: linear regression of the points, 1st, 2nd and 3rd derivative are calculated.
- If B coef of samples is negative, it means it *should* be O(1). Also check B coef of ll the other derivative which should be very low.
- If B coef of D derivative roughly equals A coef of D+1 derivative, it must be a O(n^D).
- If B coef of D derivative is negative, it *should* be a O(n^D*log(n))
*/
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

void deriv(double **dx, double **dy, int *dn,
           const double *x, const double *y, int n)
{
    int i;
    *dn = n - 2;
    *dx = malloc(*dn * sizeof(double));
    *dy = malloc(*dn * sizeof(double));
    double delta = x[1] - x[0];
    for (i = 1; i < n - 1; i++)
    {
        (*dx)[i-1] = x[i];
        (*dy)[i-1] = (y[i+1] - y[i-1]) / (2 * delta);
    }
}

void deriv2(double **dx, double **dy, int *dn,
            const double *x, const double *y, int n)
{
    int i;
    *dn = n - 2;
    *dx = malloc(*dn * sizeof(double));
    *dy = malloc(*dn * sizeof(double));
    double delta = x[1] - x[0];
    for (i = 1; i < n - 1; i++)
    {
        (*dx)[i-1] = x[i];
        (*dy)[i-1] = (y[i+1] - 2 * y[i] + y[i-1]) / (delta * delta);
    }
}

void deriv3(double **dx, double **dy, int *dn,
            const double *x, const double *y, int n)
{
    int i;
    *dn = n - 4;
    *dx = malloc(*dn * sizeof(double));
    *dy = malloc(*dn * sizeof(double));
    double delta = x[1] - x[0];
    for (i = 2; i < n - 2; i++)
    {
        (*dx)[i-2] = x[i];
        (*dy)[i-2] = (y[i+2] - 2 * y[i+1] + 2 * y[i-1] - y[i-2]) / (2 * delta * delta * delta);
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
    int n, dn;  // ]5, 1000[
    double *smpX, *smpY, deltaT;
    
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
        if (i == 1)
            deltaT = smpX[i] - smpX[i - 1];
        else if (i >= 2)
            if (smpX[i] - smpX[i - 1] != deltaT)
            {
                fprintf(stderr, "%f != %f(deltatT)\n", smpX[i] - smpX[i - 1], deltaT);
                return 0;
            }

        // safety check
        /*if (smpX[i] <= 0.0 || smpX[i] > 1.0 ||
            smpY[i] <= 0.0 || smpY[i] > 1.0)
        {
            fprintf(stderr, "Value out of range: %f %f\n", smpX[i], smpY[i]);
            return -1;
        }*/
    }
    fprintf(stderr, "Delta=%f\n", deltaT);

    double *dY, *ddY, *dddY;
    double *dX, *ddX, *dddX;
    checkO1(smpX, smpY, n);
    deriv(&dX, &dY, &dn, smpX, smpY, n);
    checkO1(dX, dY, dn);
    deriv2(&ddX, &ddY, &dn, smpX, smpY, n);
    checkO1(ddX, ddY, dn);
    deriv3(&dddX, &dddY, &dn, smpX, smpY, n);
    /*for (int i = 0; i < dn; i++)
        fprintf(stderr, "d3=%f\n", dddY[i]);*/
    checkO1(dddX, dddY, dn);

    enum BigO bigO = O_1;
    
    printf("%s\n", strO[bigO]);

    return 0;
}

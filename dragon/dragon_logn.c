#include <stdio.h>
#include <stdlib.h>


/*
Optimisation of the first BigO(n) implementation of dragon_n.c.
We aim at providing a BigO(log(n)) version, and few code optimisations
here and there.
*/

//#define LIST

#ifdef LIST
#define DEBUG(FMT, ...) do { } while(0)
#else
#define DEBUG(FMT, ...) do { } while(0)
//#define DEBUG(FMT, ...) do { printf(FMT, ##__VA_ARGS__); } while(0)
#endif

// Direction in clockwise order
enum Dir
{
    UP = 0,
    LEFT,
    DOWN,
    RIGHT
};


/**
 * Number of vectors in a Heighway Dragon fractal depending on order.
 */
static long long nbVector(int order)
{
    return 1ll << order;
}

/**
 * Number of vectors in a Heighway Dragon fractal depending on order.
 */
static long long nbVector(int order)
{
    return 1ll << order;
}

/**
 * Mathematically:
 *   x = cos(order * PI / 4) * pow(sqrt(2), order)
 * It may be simplified as below.
 */
static int lastX(int order)
{
    // 3 first bits gives a multiple of 45° angle
    switch (order & 7)
    {
    case 0:
    case 4:
        return 0;
    case 1:
    case 2:
    case 3:
        return 1l << (order >> 1);
    default:    // 5 6 7
        return -(1l << (order >> 1));
    }
}

/**
 * Mathematically:
 *   y = sin(order * PI / 4) * pow(sqrt(2), order)
 * It may be simplified as below.
 */
static int lastY(int order)
{
    // 3 first bits gives a multiple of 45° angle
    switch (order & 7)
    {
    case 2:
    case 6:
        return 0;
    case 0:
    case 1:
    case 7:
        return 1l << (order >> 1);
    default:    // 3 4 5
        return -(1l << (order >> 1));
    }
}

static enum Dir rotateLeft(enum Dir dir)
{
    return (dir + 1) & 3;
}

static void applyRotation(int *x, int *y, int order, enum Dir dir)
{
    int nx, ny;

    // simplify rotation matrix
    switch (dir)
    {
    case UP:
        nx = lastX(order);
        ny = lastY(order);
        break;
    case LEFT:
        nx = -lastY(order);
        ny = lastX(order);
        break;
    case DOWN:
        nx = -lastX(order);
        ny = -lastY(order);
        break;
    case RIGHT:
        nx = lastY(order);
        ny = -lastX(order);
        break;
    default:
        nx = ny = 0;
    }
    DEBUG("[nx=%d ny=%d dir=%d] ", nx, ny, dir);
    *x += nx;
    *y += ny;
}

/**
 * Gives (x, y) coord in D(order) after nF "F" steps.
 * Return:
 * 0 if solution was successfully found.
 * -1 if arg is NULL.
 * -2 if order is negative or out of range.
 * -3 if nF is negative or out of range.
 * -4 unexpected error.
 */
int dragon_getCoordN(int *x, int *y, int order, long long nF)
{
    enum Dir dir;

    // Do all parameter checks straightaway, never to be done again
    // in the sub-routines
    if (x == NULL || y == NULL)
        return -1;
    if (order < 0 || order > 62)    // if we want x and y to be signed 32-bits
        return -2;
    if (nF < 0 || nF > (1ll << order))
        return -3;

    *x = *y = 0;

    // special case of nF = 0
    if (nF == 0)
        return 0;

    // special case of order = 0
    if (order == 0)
    {
        *x = 1;
        return 0;
    }

    dir = UP;

    while (order >= 0)
    {
        DEBUG("Loop order=%d nF=%lld\n", order, nF);
        if (nF & (1ll << order))
        {
            DEBUG("In (%d %d) => ", *x, *y);

            if (nF - (1ll << order) == 0)
            {
                applyRotation(x, y, order, dir);
                DEBUG("(%d %d)\n", *x, *y);
                return 0;
            }

            applyRotation(x, y, order + 1, dir);

            nF = (1ll << (order + 1)) - nF;
            dir = rotateLeft(dir);
            DEBUG("(%d %d)\n", *x, *y);
        }
        order--;
    }

    return -4;
}

int main(int argc, char **argv)
{
    int x = 0, y = 0, ret;
    int order;
    long long nF;

#ifndef LIST
    if (argc != 3)
        return -1;

    order = atoi(argv[1]);
    nF    = atoll(argv[2]);

    printf("Search order=%d nF=%lld\n", order, nF);

    ret = dragon_getCoordN(&x, &y, order, nF);

    printf("x=%d y=%d (ret=%d)\n", x, y, ret);
#else
    if (argc != 2)
        return -1;

    for(nF=2; nF < atoi(argv[1]); nF++)
    {
        ret = dragon_getCoordN(&x, &y, 24, nF);
        printf("x=%d y=%d\n", x, y);
    }
#endif

    return 0;
}


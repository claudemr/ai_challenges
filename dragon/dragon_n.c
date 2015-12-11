/*
gcc dragon_n.c -o dragon
*/
#include <stdio.h>
#include <stdlib.h>


//#define LIST

#ifdef LIST
#define DEBUG(FMT, ...) do { } while(0)
#else
#define DEBUG(FMT, ...) do { } while(0)
//#define DEBUG(FMT, ...) do { printf(FMT, ##__VA_ARGS__); } while(0)
#endif

int nL = 0, nR = 0;

// Direction in clockwise order
enum Dir
{
    UP = 0,
    LEFT,
    DOWN,
    RIGHT
};


static enum Dir rotate(enum Dir dir, char turn)
{
    if (turn == 'L') nL++; else nR++;
    return (dir + (turn == 'L' ? 1 : 3)) & 3;
}

static void applyDir(int *x, int *y, enum Dir dir)
{
    switch (dir)
    {
    case LEFT:
        (*x)--;
        break;
    case RIGHT:
        (*x)++;
        break;
    case UP:
        (*y)++;
        break;
    case DOWN:
        (*y)--;
        break;
    default:
        return;
    }
}

/* This is an O(n) solution, depending on nF
   It is pretty naive, unoptimized and straightforward */
static int dragon_getCoordNStr(int *x, int *y,
                               const char *str, enum Dir *dir,
                               int order, long long *nF)
{
    int ret;
    //int xTmp, yTmp;

    // input parameter check
    if (str == NULL || dir == NULL || nF == NULL)
        return -1;
    if (order < 0)
        return -2;
    if (*nF <= 0)
        return -3;

    DEBUG("\tx=%d y=%d str=%s dir=%d order=%d nF=%lld\n",
          *x, *y, str, *dir, order, *nF);

    while (*str != '\0')
    {
        switch (*str)
        {
        case 'F':
            // Move foward depending on the direction
            applyDir(x, y, *dir);
            (*nF)--;
            DEBUG("\t\tF (%d, %d)\n", *x, *y);
            // Have we found it?
            if (*nF == 0)
            {
                DEBUG("\t\tFound! x=%d y=%d\n", *x, *y);
                return 1;
            }
            break;

        case 'L':
        case 'R':
            // Turn left/right
            DEBUG("\t\t%c\n", *str);
            *dir = rotate(*dir, *str);
            break;

        case 'a':
            // If order
            if (order == 0)
                break;
            ret = dragon_getCoordNStr(x, y, "aRbFR", dir, order - 1, nF);
            if (ret != 0)
                return ret;
            break;
        case 'b':
            if (order == 0)
                break;
            ret = dragon_getCoordNStr(x, y, "LFaLb", dir, order - 1, nF);
            if (ret != 0)
                return ret;
            break;
        default:
            return -4;
        }
        str++;
    }

    // Have not found the solution yet
    return 0;
}

/**
 * Gives (x, y) coord in D(order) after nF "F" steps.
 * Return:
 * 1 if solution was successfully found.
 * 0 if nF is too big according to order
 * -1 if arg is NULL.
 * -2 if order is negative.
 * -3 if nF is negative or 0.
 * -4 if unexpected error.
 */
int dragon_getCoordN(int *x, int *y, int order, long long nF)
{
    enum Dir dir;
    if (x == NULL || y == NULL)
        return -1;

    *x = *y = 0;
    dir = UP;

    return dragon_getCoordNStr(x, y, "Fa", &dir, order, &nF);
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
    printf("nL=%d nR=%d\n", nL, nR);
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


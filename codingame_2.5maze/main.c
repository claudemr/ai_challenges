#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define DIR_L 0
#define DIR_R 1
#define DIR_U 2
#define DIR_D 3
#define NB_DIR 4

struct MazeNode
{
    char             c;
    int              x, y;
    int              level;
    struct MazeNode *next[NB_DIR];
    struct MazeNode *prev;
};

static int goDir(int *nx, int *ny, int x, int y, int dir, int w, int h)
{
    switch (dir)
    {
    case DIR_L:
        x--;
        break;
    case DIR_R:
        x++;
        break;
    case DIR_U:
        y--;
        break;
    case DIR_D:
        y++;
        break;
    default:
        return -1;
    }
    
    if (x < 0 || x >= w)
        return -2;

    if (y < 0 || y >= h)
        return -3;

    *nx = x;
    *ny = y;
    return 0;
}

static int checkStepBack(struct MazeNode *treeMaze, int x, int y, int level)
{
    if (treeMaze->prev != NULL)
    {
        // If it is the same position
        if (x == treeMaze->prev->x && y == treeMaze->prev->y)
        {
            // If it is the same level (special bridge case)
            if (level == treeMaze->prev->level)
                return -1;
        }
        
        checkStepBack(treeMaze->prev, x, y, c);
    }
    
    return 0;
}

static void initMazeTree(struct MazeNode *treeMaze,
                         const char *maze, int x, int y, int w)
{
    memset(treeMaze, 0x0, sizeof(struct TreeMaze));
    treeMaze->x = x;
    treeMaze->y = y;
    treeMaze->c = maze[x + y * w];
}

static void buildMazeTree(struct MazeNode *treeMaze,
                          const char *maze, int w, int h)
{
    int d;

    for (d = 0; d < NB_DIR; d++)
    {
        int nx, ny, ret;
        
        // Cannot go L/R on vertical slope
        if (treeMaze->c == '|' && (d == DIR_L || d == DIR_R)
            continue;
        // Cannot go U/D on horizontal slope
        if (treeMaze->c == '-' && (d == DIR_U || d == DIR_D)
            continue;
        
        ret = goDir(&nx, &ny, treeMaze->x, treeMaze->y, d, w, h);
        
        // If cannot move, next direction
        if (ret != 0)
            continue;

        // Don't step back
        if (checkStepBack(treeMaze, nx, ny) != 0)
            continue;
        
        // Cannot go on high wall
        if (maze[nx + ny * w] == '#')
            continue;
        
        // If on the floor...
        if (treeMaze->c == '.')
        {
            // Cannot go straight on short wall
            if (maze[nx + ny * w] == '+')
                continue;
        }

        // If on a short wall...
        if (treeMaze->c == '+')
        {
            // Cannot go straight on the floor
            if (maze[nx + ny * w] == '.')
                continue;
        }

        // Cannot cross slopes
        if (maze[nx + ny * w] == '|' && (d == DIR_L || d == DIR_R)
            continue;
        if (maze[nx + ny * w] == '-' && (d == DIR_U || d == DIR_D)
            continue;

        // A level 1 slope cannot lead to another short wall (different level of short walls not permitted)
        if ((treeMaze->c == '|' || treeMaze->c == '-') &&
            treeMaze->level == 1 &&
            maze[nx + ny * w] == '+')
            continue;
        // A level 0 slope cannot lead to another floor (different level of floors not permitted)
        if ((treeMaze->c == '|' || treeMaze->c == '-') &&
            treeMaze->level == 0 &&
            maze[nx + ny * w] == '.')
            continue;

        // Level 0 bridges cannot lead to shorts walls
        if (treeMaze->c == 'X' && treeMaze->level == 0 && maze[nx + ny * w] == '+')
            continue;
        // Level 1 bridges cannot lead to floors
        if (treeMaze->c == 'X' && treeMaze->level == 1 && maze[nx + ny * w] == '.')
            continue;

        // We can go in that direction now!
        // So allocate new tree node
        treeMaze->next[d] = calloc(1, sizeof(struct MazeNode));
        treeMaze->next[d]->c = maze[nx + ny * w];
        treeMaze->next[d]->x = nx;
        treeMaze->next[d]->y = ny;
        treeMaze->next[d]->prev = treeMaze;
        if (treeMaze->c == '|' || treeMaze->c == '-')
            treeMaze->next[d]->level = 1 - treeMaze->level;
        else
            treeMaze->next[d]->level = treeMaze->level;
    }
}

// To debug: fprintf(stderr, "Debug messages...\n");
int main()
{
    // Parse input
    int starty;
    int startx;
    scanf("%d%d", &starty, &startx);
    int endy;
    int endx;
    scanf("%d%d", &endy, &endx);
    int h;
    int w;
    scanf("%d%d", &h, &w);
    char* maze = malloc(h * w * sizeof(char));
    for (int i = 0; i < h; i++)
        scanf("%s", &maze[i * w]);
    
    struct TreeMaze treeMaze;
    initMazeTree(&treeMaze, maze, startx, starty, w);
    buildMazeTree(&treeMaze, maze, w, h);

    printf("%d\n", n);

    return 0;
}

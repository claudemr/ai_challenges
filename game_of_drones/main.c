#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
/*

gcc main.c -Wall -o game

*/

/********************************* LIST ******************************/

struct list
{
    struct list *next;
    struct list *prev;
};

void list_init(struct list *list)
{
    list->next = list->prev = list;
}

bool list_is_null_link(const struct list *list)
{
    return list->prev == NULL && list->next == NULL;
}

bool list_is_empty(const struct list *list)
{
    return list->prev == list && list->next == list;
}

int list_nb_link(const struct list *list)
{
    int n = 0;
    struct list *lnk;
    for (lnk = list->next; lnk != list; lnk = lnk->next)
        n++;
    return n;
}

void list_add_next(struct list *list, struct list *link)
{
    link->prev = list;
    link->next = list->next;
    link->next->prev = link;
    list->next = link;
}

void list_add_prev(struct list *list, struct list *link)
{
    link->next = list;
    link->prev = list->prev;
    link->prev->next = link;
    list->prev = link;
}

void list_del(struct list *link)
{
    link->next->prev = link->prev;
    link->prev->next = link->next;
    link->next = link->prev = NULL;
}

void list_empty(struct list *list)
{
    while(!list_is_empty(list))
        list_del(list->next);
}

#define list_walk_player_zones(player, zone) \
            for ((zone) = (struct zone *)(void *)(player)->list_ctrl_zones.next; \
                 (zone) != (struct zone *)(void *)&(player)->list_ctrl_zones; \
                 (zone) = (struct zone *)(void *)(zone)->link_player.next)
#define list_walk_zone_drones(zone, player, drone) \
            for ((drone) = (struct drone *)(void *)(zone)->list_drones[player].next; \
                 (drone) != (struct drone *)(void *)&(zone)->list_drones[player]; \
                 (drone) = (struct drone *)(void *)(drone)->link_zone.next)


/********************************* TYPEDEF ******************************/

#define MAX_PLAYER              4
#define MAX_ZONE                8
#define MAX_DRONE_PER_PLAYER    11
#define ZONE_RADIUS     100
#define ZONE_RADIUS2    (ZONE_RADIUS * ZONE_RADIUS)
#define MAX_DIST        100
#define MAX_DIST2       (MAX_DIST * MAX_DIST)
#define MAX_X           4000
#define MAX_Y           1800
#define MAX_LAYER       (MAX_X / MAX_DIST)
#define BIGGEST_DIST2   (MAX_X * MAX_Y + 1)

#define TURNS_PER_ROUND 50


struct coord
{
    int x, y;
};

struct player;

struct zone
{
    struct list     link_player;
    struct coord    coord;
    bool            targeted;
    int             nb_layer;
    struct
    {
        int force[MAX_PLAYER];
        int enemy_force;	// best enemy force
        int target;		// nb of drones required to secure the layer
    }               layers[MAX_LAYER];
    struct player   *ctrl_player;
};

struct drone
{
    struct coord    coord;
    struct coord    target_coord;
    struct zone     *ctrl_zone;
    struct player   *player;
};

struct player
{
    int             id;
    struct drone    *drones;
    struct list     list_ctrl_zones; // list of zones controlled by player
    int             points;          // nb of points made so far
    int             points_round;    // nb of points made in the round
};


/********************************* FUNCTIONS ******************************/

/* squared distance between 2 points */
int distance2(struct coord *c0, struct coord *c1)
{
    int dx, dy;
    dx = c0->x - c1->x;
    dy = c0->y - c1->y;
    return dx * dx + dy * dy;
}


/********************************* GLOBAL DATA ******************************/

int g_my_id;                // [0, 3]
int g_nb_players;           // [2, 4]
int g_nb_zones;             // [4, 8]
int g_nb_drones_per_player; // [3, 11]
int g_nb_turns;
int g_nb_rounds;
int g_nb_target_zones;
struct player *g_players;
struct zone   *g_zones;
struct drone  *g_drones;
struct player *g_my_player;

/********************************* MAIN ******************************/


static void move_drones(void);
static void make_stat(void);
static void clear_stat(void);
static void make_round_strategy(void);
static void choose_nb_target_zone(void);

int main(void)
{
    int i, p;

    // Read init information from standard input
    scanf("%d %d %d %d", &g_nb_players, &g_my_id,
          &g_nb_drones_per_player, &g_nb_zones);

    g_players = calloc(1, sizeof(struct player) * g_nb_players);
    g_zones   = calloc(1, sizeof(struct zone)   * g_nb_zones);
    g_drones  = calloc(1, sizeof(struct drone)  * g_nb_players * g_nb_drones_per_player);

    g_my_player = &g_players[g_my_id];

    for (i = 0; i < g_nb_players; i++)
    {
        g_players[i].id = i;
        g_players[i].drones = g_drones + i * g_nb_drones_per_player;
        list_init(&g_players[i].list_ctrl_zones);
    }

    for (i = 0, p = 0; i < g_nb_players * g_nb_drones_per_player; i++)
    {
        if (i >= g_nb_drones_per_player * (p + 1))
            p++;
        g_drones[i].player = &g_players[p];
    }

    for (i = 0; i < g_nb_zones; i++)
        scanf("%d %d", &g_zones[i].coord.x, &g_zones[i].coord.y);

    g_nb_turns = g_nb_rounds = 0;
    make_round_strategy();

    fprintf(stderr, "nz=%d np=%d ndpp=%d id=%d ntz=%d\n",
            g_nb_zones, g_nb_players, g_nb_drones_per_player, g_my_id, g_nb_target_zones);

    while (1)
    {
        // Read zone control
        for (i = 0; i < g_nb_zones; i++)
        {
            scanf("%d", &p);
            if (p != -1)
            {
                g_zones[i].ctrl_player = &g_players[p];
                list_add_next(&g_players[p].list_ctrl_zones, &g_zones[i].link_player);
                g_players[p].points++;
            }
            else
                g_zones[i].ctrl_player = NULL;
        }

        // Read drones coords
        for (i = 0; i < g_nb_players * g_nb_drones_per_player; i++)
            scanf("%d %d", &g_drones[i].coord.x, &g_drones[i].coord.y);

        make_stat();

        // Compute logic here
        move_drones();

        // Write action to standard output
        for (i = 0; i < g_nb_drones_per_player; i++)
        {
            printf("%d %d\n",
                   g_my_player->drones[i].target_coord.x,
                   g_my_player->drones[i].target_coord.y);
        }

        clear_stat();
        g_nb_turns++;
        if (g_nb_turns >= TURNS_PER_ROUND)
        {
            g_nb_rounds++;
            make_round_strategy();
        }
    }

    return 0;
}

static void make_round_strategy(void)
{
    int p, z;
    // recalculate the number of target zones
    choose_nb_target_zone();

    //xxx
    for (z = 0; z < g_nb_target_zones; z++)
        g_zones[z].targeted = true;

    for (p = 0; p < g_nb_players; p++)
        g_players[p].points_round = 0;
}

static void choose_nb_target_zone(void)
{
    g_nb_target_zones = g_nb_zones / 2 + 1;
}

void find_layer_best_enemy_force(struct zone *zone, int layer)
{
    int p, max = 0;

    for (p = 0; p < g_nb_players; p++)
        if (p != g_my_id && zone->layers[layer].force[p] > max)
            max = zone->layers[layer].force[p];

    zone->layers[layer].enemy_force = max;
}

static int dist2_to_layer(int dist2)
{
    int l;

    for (l = 0; l < MAX_LAYER; l++)
        if ((uint64_t)dist2 <= (uint64_t)MAX_DIST2 * l * l)
            return l;

    return MAX_LAYER-1;
}

static void make_stat(void)
{
    int d, z, p, dist2, l, max_l;

    for (z = 0; z < g_nb_zones; z++)
    {
        max_l = 0;
        for (d = 0, p = 0; d < g_nb_players * g_nb_drones_per_player; d++)
        {
            if (d >= g_nb_drones_per_player * (p + 1))
                p++;
            dist2 = distance2(&g_drones[d].coord, &g_zones[z].coord);
            l = dist2_to_layer(dist2);
            if (l > max_l)
                max_l = l;
            g_zones[z].layers[l].force[p]++;
            //fprintf(stderr, "Force++ for p=%d on z=%d at l=%d\n", p, z, l);
        }
        g_zones[z].nb_layer = max_l;

        // first layer target check
        if (!g_zones[z].targeted)
            continue;

        find_layer_best_enemy_force(&g_zones[z], 0);

        // if this zone is controlled by no-one, target it
        if (g_zones[z].ctrl_player == NULL)
            g_zones[z].layers[0].target = 1;
        else if (g_zones[z].ctrl_player != g_my_player)
            g_zones[z].layers[0].target = g_zones[z].layers[0].enemy_force + 1;
        else
            g_zones[z].layers[0].target = g_zones[z].layers[0].enemy_force;

        for (l = 1; l < g_zones[z].nb_layer; l++)
        {
            find_layer_best_enemy_force(&g_zones[z], l);

            if (g_zones[z].layers[l].enemy_force > 0)
                g_zones[z].layers[l-1].target += g_zones[z].layers[l].enemy_force;
        }

        /*fprintf(stderr, "Enemy force=%d target=%d on z=%d (p=%d) at l=0\n",
                g_zones[z].layers[0].enemy_force, g_zones[z].layers[0].target, z,
                g_zones[z].ctrl_player == NULL ? -1 : g_zones[z].ctrl_player->id);*/
    }
}

static void clear_stat(void)
{
    int p, z, d, l;

    for (z = 0; z < g_nb_zones; z++)
        for (l = 0; l < MAX_LAYER; l++)
        {
            for (p = 0; p < g_nb_players; p++)
                g_zones[z].layers[l].force[p] = 0;
            g_zones[z].layers[l].enemy_force = 0;
            g_zones[z].layers[l].target = 0;
        }

    for (p = 0; p < g_nb_players; p++)
        list_empty(&g_players[p].list_ctrl_zones);
    for (d = 0; d < g_nb_players * g_nb_drones_per_player; d++)
        g_drones[d].ctrl_zone = NULL;
    for (d = 0; d < g_nb_drones_per_player; d++)
        g_my_player->drones[d].target_coord = g_my_player->drones[d].coord;
}

static void move_drones(void)
{
    int d, z, layer, dist2, dist2_to_target;
    struct zone *targeted_zone;

    layer = 0;

    for (d = 0; d < g_nb_drones_per_player; d++)
    {
        targeted_zone = NULL;
        dist2_to_target = BIGGEST_DIST2;

next_layer:
        for (z = 0; z < g_nb_zones; z++)
        {
            if (!g_zones[z].targeted || g_zones[z].nb_layer < layer)
                continue;
            // if it requires a target, check its distance
            if (g_zones[z].layers[layer].target > 0)
            {
                dist2 = distance2(&g_my_player->drones[d].coord, &g_zones[z].coord);
                if (dist2 < dist2_to_target)
                {
                    dist2_to_target = dist2;
                    targeted_zone = &g_zones[z];
                }
            }
        }
        // if no target found, try next layer
        if (targeted_zone == NULL)
        {
            layer++;
            goto next_layer;
        }
        else
        {
            g_my_player->drones[d].target_coord = targeted_zone->coord;
            targeted_zone->layers[layer].target--;
        }
    }
}

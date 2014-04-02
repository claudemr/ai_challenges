#include <stdlib.h>
#include <stdio.h>

/***************************************** MACRO **************************************/

#define NEW(ptr) do {  ptr = calloc(1, sizeof(*ptr)); \
                            if (ptr == NULL) \
                                fprintf(stderr, "Not enough memory for %dbytes. " \
                                                "%dbytes already allocated\n", \
                                        sizeof(*ptr), g_alloc_size); \
                            g_alloc_size += sizeof(*ptr); \
                        } while(0)

#define DELETE(ptr) do { free(ptr); g_alloc_size -= sizeof(*ptr); } while (0)

/***************************************** TYPEDEF **************************************/

struct time_slice;

struct request_slice
{
    int                  id;
    struct request_slice *next_own_slice;
    struct time_slice    *time_slice;
    struct request_slice *next_concurrent_request;
}

struct time_slice
{
    int start;
    int duration;
    struct request_slice *requests; // list of requests on that time slice
    struct time_slice    *next_slice;
    struct time_slice    *prev_slice;
};

/***************************************** GLOBAL **************************************/

int g_alloc_size = 0;
int g_nb_request = 0;
struct time_slice *g_time_slices = NULL;

/***************************************** FUNCTION **************************************/

struct request_slice *request_slice_insert(int id, struct request_slice *list_request_slice,
                                           struct request_slice *prev_request_slice)
{
    struct request_slice *req;
    NEW(req);
    if (req == NULL)
        return NULL;
    req->id = id;
    req->next_concurrent_request = list_request_slice;
    if (prev_request_slice != NULL)
        prev_request_slice->next_own_slice = req;
    return req;
}

struct time_slice *time_slice_cut(struct time_slice *time_slice,
                                  int duration)
{
    struct time_slice *new_time_slice;
    struct request_slice *request_slice, *nrs;
    NEW(new_time_slice);
    if (new_time_slice == NULL)
        return NULL;
    *prev_time_slice = new_time_slice;
    new_time_slice->start      = time_slice->start;
    new_time_slice->duration   = duration;
    new_time_slice->next_slice = time_slice;
    new_time_slice->prev_slice = time_slice->prev;
    time_slice->prev = time_slice;
    new_time_slice->prev_slice->next_slice = new_time_slice;
    time_slice->start    = new_time_slice->start + new_time_slice->duration;
    time_slice->duration -= duration;
    
    for (request_slice = time_slice->requests;
         request_slice != NULL;
         request_slice = request_slice->next_concurrent_request);
    {
        nrs = request_slice_insert(request_slice->id, new_time_slice->requests, NULL);
        if (nrs == NULL)
            return NULL;
        nrs->time_slice = new_time_slice;
        nrs->next_own_slice = request_slice;
        new_time_slice->requests = nrs;
    }

    for (request_slice = time_slice->prev_slice->requests;
         request_slice != NULL;
         request_slice = request_slice->next_concurrent_request);
    {
        for (nrs = time_slice->requests; nrs != NULL; nrs = nrs->next_concurrent_request)
            if (request_slice->id == nrs->id)
                request_slice->next_own_slice = nrs;
    }
    
}


struct time_slice *time_slice_new(int start, int duration, int request_id,
                                  struct request_slice *prev_request_slice)
{
    struct time_slice *time_slice;
    NEW(time_slice);
    if (time_slice == NULL)
        return NULL;
    time_slice->start    = start;
    time_slice->duration = duration;
    time_slice->requests = request_slice_insert(request_id, NULL, prev_request_slice);
    if (time_slice->requests == NULL)
    {
        free(time_slice);
        return NULL;
    }
    return time_slice;
}

int slice_insert(int request_id, int start, int duration,
                 struct time_slice *time_slice,
                 struct request_slice *prev_request_slice)
{
    int ret;
    struct time_slice *ts, *nts;
    
    // else, walk through the slices to see where the new slice should lie
    for (ts = time_slice; ts != NULL; ts = ts->next_slice)
    {
        if (start < ts->start)
        {
            // it fits just before this time-slice
            if (start + duration <= ts->start)
            {
                nts = time_slice_new(start, duration, request_id, prev_request_slice);
                if (nts == NULL)
                    return -2;
                nts->next_slice = ts;
                nts->prev_slice = ts->prev_slice;
                ts->prev_slice  = nts;
                nts->prev_slice->next_slice = nts;
                return 0;
            }
            // first bit is free but not all
            else
            {
                // allocate first bit
                nts = time_slice_new(start, ts->start - start, request_id, NULL);
                if (nts == NULL)
                    return -3;
                nts->next_slice = ts;
                nts->prev_slice = ts->prev_slice;
                ts->prev_slice  = nts;
                nts->prev_slice->next_slice = nts;
                // and insert next bit
                ret = slice_insert(request_id, ts->start, duration - (ts->start - start),
                                   nts->next_slice, nts->requests);
                return ret;
            }
        }
        // it interleaves with this time-slice
        else
        {
            // proper share of time-slice
            if (start == ts->start)
            {
                // exact or more duration
                if (duration >= ts->duration)
                {
                    // simply insert new request
                    struct request_slice *rs;
                    rs = request_slice_insert(request_id, ts->requests, prev_request_slice);
                    if (rs == NULL)
                        return -4;
                    ts->requests = rs;
                    rs->time_slice = ts;
                    
                    if (duration > ts->duration)
                    {
                        // and insert next bit
                        ret = slice_insert(request_id, ts->start + ts->duration - 1,
                                           duration - ts->duration,
                                           ts->next_slice, rs);
                        return ret;
                    }
                    else
                        return 0;
                }
                // duration is lower
                else
                {
                    nts = time_slice_cut(ts, duration);
                    if (nts == NULL)
                        return -5;
                    rs = request_slice_insert(request_id, nts->requests, prev_request_slice);
                    if (rs == NULL)
                        return -6;
                    nts->requests = rs;
                    rs->time_slice = nts;
                    return 0;
                }
            }
            // have to slice the current time-slice
            else if (start < ts->start + ts->duration)
            {
                nts = time_slice_cut(ts, start - ts->start);
                if (nts == NULL)
                    return -7;
                // and insert next bit
                ret = slice_insert(request_id, start,
                                   duration,
                                   nts->next_slice, NULL);
                return ret;
            }
        }
        // check next slice
    }
}

/***************************************** MAIN **************************************/

int main()
{
    int i, ret, start, duration;

    //  get number of requests    
    ret = scanf("%d", &g_nb_request);
    if (ret != 1)
    {
        fprintf(stderr, "Wrong first input\n");
        return -1;
    }
    
    if (g_nb_request <= 0)
    {
        fprintf(stderr, "Wrong nb of requests %d\n", g_nb_request);
        return -2;
    }

    // get first line
    ret = scanf("%d %d", &start, &duration);
    if (ret != 2)
    {
        fprintf(stderr, "Wrong second input\n");
        return -3;
    }
    g_time_slices = time_slice_new(start, duration, 0, NULL);
    if (g_time_slices == NULL)
        return -4;

    // get all the other lines
    for (i = 1; i < g_nb_request; i++)
    {
        ret = scanf("%d %d", &start, &duration);
        if (ret != 2)
        {
            fprintf(stderr, "Wrong input\n");
            return -10;
        }
        
        ret = slice_insert(i, start, duration, g_time_slices, NULL);
        if (ret != 0)
        {
            fprintf(stderr, "Could not create new slice: ret=%d\n", ret);
            return -11;
        }
    }
    
    // walk though the time-slices to find the max number of requests
    
    return 0;
}

#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

// Minimum of three elements !
// int Mini(int a, int b, int c)
// {
//     if ( (a<b) && (a<c) ){
//         return a;
//     }
//     else if ( (b<a) && (b<c) ){
//         return b;
//     }
//     else{
//         return c;
//     }
// }

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args)
{
    struct thread *thread_with_earliest_deadline = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_earliest_deadline == NULL){
            thread_with_earliest_deadline = th;
        }
        else if (thread_with_earliest_deadline->current_deadline <= args.current_time){
            if (th->ID < thread_with_earliest_deadline->ID){
                thread_with_earliest_deadline = th;
            }
        }
        else if ( (th->current_deadline == thread_with_earliest_deadline->current_deadline && th->ID < thread_with_earliest_deadline->ID) || (th->current_deadline < thread_with_earliest_deadline->current_deadline) ){
            thread_with_earliest_deadline = th;
        }
    }



    struct threads_sched_result r;

    if (thread_with_earliest_deadline == NULL){
        struct release_queue_entry *higher_rqe = NULL;
        struct release_queue_entry *rqe = NULL;
        int release_time;

        list_for_each_entry(rqe, args.release_queue, thread_list){
            if (higher_rqe == NULL){
                higher_rqe = rqe;
            }
            else if (rqe->release_time < higher_rqe->release_time){
                higher_rqe = rqe;
            }
        }
        release_time = higher_rqe->release_time;
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = release_time-args.current_time;
    }

    else if (thread_with_earliest_deadline->current_deadline <= args.current_time){
        r.scheduled_thread_list_member = &thread_with_earliest_deadline->thread_list;
        r.allocated_time = 0;
    }

    else{
        struct release_queue_entry *higher_rqe = NULL;
        struct release_queue_entry *rqe = NULL;

        // int allo_time = thread_with_earliest_deadline->remaining_time;
        // if(thread_with_earliest_deadline->current_deadline - args.current_time < allo_time) {
        //     allo_time = thread_with_earliest_deadline->current_deadline - args.current_time;
        // }

        // list_for_each_entry(rqe, args.release_queue, thread_list) {
        //     if (rqe->release_time + rqe->thrd->period == thread_with_earliest_deadline->current_deadline && rqe->thrd->ID < thread_with_earliest_deadline->ID) {
        //         if(rqe->release_time - args.current_time < allo_time) {
        //             allo_time = rqe->release_time - args.current_time;
        //         }
        //     }
        //     else if (rqe->release_time + rqe->thrd->period < thread_with_earliest_deadline->current_deadline){
        //         if(rqe->release_time - args.current_time < allo_time) {
        //             allo_time = rqe->release_time - args.current_time;
        //         }
        //     }
        // }

        // int allo_time = 0;
        list_for_each_entry(rqe, args.release_queue, thread_list) {
            if (rqe->release_time + rqe->thrd->period == thread_with_earliest_deadline->current_deadline && rqe->thrd->ID < thread_with_earliest_deadline->ID) {
                if(rqe->release_time < higher_rqe->release_time) {
                    higher_rqe = rqe;
                }
            }
            else if (rqe->release_time + rqe->thrd->period < thread_with_earliest_deadline->current_deadline){
                if(rqe->release_time < higher_rqe->release_time) {
                    higher_rqe = rqe;
                }
            }
        }

        int allo_time = higher_rqe->release_time - args.current_time;
        if (thread_with_earliest_deadline->remaining_time < allo_time){
            allo_time = thread_with_earliest_deadline->remaining_time;
        }
        if (thread_with_earliest_deadline->current_deadline - args.current_time < allo_time){
            allo_time = thread_with_earliest_deadline->current_deadline - args.current_time;
        }

        r.allocated_time = allo_time;
        r.scheduled_thread_list_member = &thread_with_earliest_deadline->thread_list;
    }

    return r;
    
    // return schedule_default(args);
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args)
{
    struct thread *thread_with_highest_rate = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_highest_rate == NULL){
            thread_with_highest_rate = th;
        }
        else if (thread_with_highest_rate->current_deadline <= args.current_time){
            if (th->ID < thread_with_highest_rate->ID){
                thread_with_highest_rate = th;
            }
        }
        else if ( (th->period == thread_with_highest_rate->period && th->ID < thread_with_highest_rate->ID) || (th->period < thread_with_highest_rate->period) ){
            thread_with_highest_rate = th;
        }
    }

    struct threads_sched_result r;

    if (thread_with_highest_rate == NULL){
        struct release_queue_entry *higher_rqe = NULL;
        struct release_queue_entry *rqe = NULL;
        int release_time;

        list_for_each_entry(rqe, args.release_queue, thread_list){
            if (higher_rqe == NULL){
                higher_rqe = rqe;
            }
            else if (rqe->release_time < higher_rqe->release_time){
                higher_rqe = rqe;
            }
        }
        release_time = higher_rqe->release_time;
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = release_time-args.current_time;
    }

    else if (thread_with_highest_rate->current_deadline <= args.current_time){
        r.scheduled_thread_list_member = &thread_with_highest_rate->thread_list;
        r.allocated_time = 0;
    }

    else{
        struct release_queue_entry *higher_rqe = NULL;
        struct release_queue_entry *rqe = NULL;
        
        list_for_each_entry(rqe, args.release_queue, thread_list) {
            if (rqe->thrd->period == thread_with_highest_rate->period && rqe->thrd->ID < thread_with_highest_rate->ID){
                if(rqe->release_time < higher_rqe->release_time){
                    higher_rqe = rqe;
                }
            }

            else if (rqe->thrd->period < thread_with_highest_rate->period){
                if(rqe->release_time < higher_rqe->release_time){
                    higher_rqe = rqe;
                }
            }
        }

        int allo_time = higher_rqe->release_time - args.current_time;
        if (thread_with_highest_rate->remaining_time < allo_time){
            allo_time = thread_with_highest_rate->remaining_time;
        }
        if (thread_with_highest_rate->current_deadline - args.current_time < allo_time){
            allo_time = thread_with_highest_rate->current_deadline - args.current_time;
        }

        // struct release_queue_entry *rqe = NULL;

        // int allo_time = thread_with_highest_rate->remaining_time;
        // if(thread_with_highest_rate->current_deadline - args.current_time < allo_time) {
        //     allo_time = thread_with_highest_rate->current_deadline - args.current_time;
        // }

        // list_for_each_entry(rqe, args.release_queue, thread_list) {
        //     if (rqe->thrd->period == thread_with_highest_rate->current_deadline && rqe->thrd->ID < thread_with_highest_rate->ID) {
        //         if(rqe->release_time - args.current_time < allo_time) {
        //             allo_time = rqe->release_time - args.current_time;
        //         }
        //     }
        //     else if (rqe->thrd->period < thread_with_highest_rate->current_deadline){
        //         if(rqe->release_time - args.current_time < allo_time) {
        //             allo_time = rqe->release_time - args.current_time;
        //         }
        //     }
        // }

        r.allocated_time = allo_time;
        r.scheduled_thread_list_member = &thread_with_highest_rate->thread_list;
    }

    return r;
    // return schedule_default(args);
}

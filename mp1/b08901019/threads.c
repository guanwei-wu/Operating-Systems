#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0


static struct thread* current_thread = NULL;
static int id = 1;

static struct thread* temp_thread = NULL;
static jmp_buf env_ret;

// static jmp_buf env_st;
static jmp_buf env_tmp;
static jmp_buf handler_env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;

    unsigned long handler_new_stack_p;
    unsigned long handler_new_stack;
    handler_new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    handler_new_stack_p = handler_new_stack +0x100*8-0x2*8;
    t->handler_stack = (void*) handler_new_stack;
    t->handler_stack_p = (void*) handler_new_stack_p;

    id++;



    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    t->to_be_killed = 0;
    t->to_be_handled = 0;
    return t;
}

void thread_add_runqueue(struct thread *t){
    if (current_thread == NULL){
        // TODO
        current_thread = t;
        current_thread->previous = t;
        current_thread->next = t;
    }
    else{
        // TODO
        current_thread->previous->next = t;
        t->previous = current_thread->previous;
        current_thread->previous = t;
        t->next = current_thread;



        // Part 2
        t->signo = current_thread->signo;
        t->sig_handler[0] = current_thread->sig_handler[0];
        t->sig_handler[1] = current_thread->sig_handler[1];
    }



    // Part 2
    // t->signo = current_thread->signo;
    // t->sig_handler[0] = current_thread->sig_handler[0];
    // t->sig_handler[1] = current_thread->sig_handler[1];
}
void thread_yield(void){
    // Part 2
    if (current_thread->to_be_killed == 1 || current_thread->to_be_handled == 1){
        if ( !setjmp(current_thread->handler_env) ){
            schedule();
            dispatch();
        }
    }



    // TODO (Part 2 + else)
    else{
        if ( !setjmp(current_thread->env) ){
            schedule();
            dispatch();
        }
    }
}
void dispatch(void){
    // Part 2
    if (current_thread->to_be_killed == 1){
        current_thread->to_be_killed = 0;
        thread_exit();
    }
    else if (current_thread->to_be_handled == 1){

        if ( current_thread->handler_buf_set == 0 ){
            current_thread->handler_buf_set = 1;
            if ( !setjmp(handler_env_tmp) ){
                handler_env_tmp->sp = ( unsigned long ) ( current_thread->handler_stack_p );
                longjmp(handler_env_tmp, 1);
            }
            ( current_thread->sig_handler[(current_thread->signo)] ) ( current_thread->signo );
        }
        else{
            longjmp(current_thread->handler_env, 1);
        }

        current_thread->to_be_handled = 0;

    }



    // TODO
    if ( current_thread->buf_set == 0 ){
        // x executed before !
        current_thread->buf_set = 1;

        // Initialization !
        if ( !setjmp(env_tmp) ){
            env_tmp->sp = ( unsigned long ) ( current_thread->stack_p );
            longjmp(env_tmp, 1);
        }

        ( current_thread->fp ) ( current_thread->arg );
    }
    else{
        // o executed before !
        longjmp(current_thread->env, 1);
    }

    thread_exit();
}
void schedule(void){
    // TODO
    current_thread = current_thread->next;
}
void thread_exit(void){
    if ( current_thread->next != current_thread ){
        // TODO
        current_thread->previous->next = current_thread->next;
        current_thread->next->previous = current_thread->previous;

        temp_thread = current_thread;
        current_thread = current_thread->next;

        free( temp_thread->handler_stack_p );
        free( temp_thread->handler_stack );

        free( temp_thread->stack_p );
        free( temp_thread->stack );
        free( temp_thread );

        dispatch();
    }
    else{
        // TODO
        // Hint: No more thread to execute
        longjmp(env_ret, 1);
    }
}
void thread_start_threading(void){
    // TODO
    if ( !setjmp(env_ret) ){
        schedule();
        dispatch();
    }
}



// part 2
void thread_register_handler(int signo, void (*handler)(int)){
    // TODO
    // current_thread->signo = signo;
    if (signo == 0){
        current_thread->sig_handler[0] = handler;
        // current_thread->sig_handler[1] = NULL_FUNC;
    }
    else{
        // current_thread->sig_handler[0] = NULL_FUNC;
        current_thread->sig_handler[1] = handler;
    }
}
void thread_kill(struct thread *t, int signo){
    t->signo = signo;
    // TODO
    if (t->sig_handler[signo] == NULL_FUNC){
        t->to_be_killed = 1;
        t->to_be_handled = 0;
    }
    else{
        t->to_be_killed = 0;
        t->to_be_handled = 1;
    }
}
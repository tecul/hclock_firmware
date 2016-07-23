#include <stdlib.h>
#include <assert.h>

#include "scheduler.h"
#include "utils.h"
#include "irq.h"
#include "systick.h"

#define TASK_NB                                                         32

struct task {
    struct task *next;
    uint32_t when;
    task_handler handler;
    uint32_t p0;
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    struct alarm alarm;
};

static struct task tasks[TASK_NB];

static struct task *free_task_list_head = NULL;
static struct task *free_task_list_queue = NULL;
static struct task *runnable_task_list_head = NULL;
static struct task *runnable_task_list_queue = NULL;

static void wfi() {
    asm volatile("wfi");
}

static void insert_free_task(struct task *task) {
    int irq_state = disable_irq();

    if (free_task_list_queue)
        free_task_list_queue->next = task;
    else
        free_task_list_head = task;
    free_task_list_queue = task;
    restore_irq(irq_state);
}

static struct task *get_free_task() {
    struct task *res;
    int irq_state = disable_irq();

    res = free_task_list_head;
    if (res) {
        free_task_list_head = free_task_list_head->next;
        if (free_task_list_head == NULL)
            free_task_list_queue = NULL;
        res->next = NULL;
    }
    restore_irq(irq_state);

    return res;
}

static void insert_runnable_task(struct task *task) {
    int irq_state = disable_irq();

    if (runnable_task_list_queue)
        runnable_task_list_queue->next = task;
    else
        runnable_task_list_head = task;
    runnable_task_list_queue = task;
    restore_irq(irq_state);
}

static struct task *get_runnable_task() {
    struct task *res;
    int irq_state = disable_irq();

    res = runnable_task_list_head;
    if (res) {
        runnable_task_list_head = runnable_task_list_head->next;
        if (runnable_task_list_head == NULL)
            runnable_task_list_queue = NULL;
        res->next = NULL;
    }
    restore_irq(irq_state);

    return res;
}

static void schedule_later_handler(struct alarm *alarm) {
    struct task *task = container_of(alarm, struct task, alarm);

    insert_runnable_task(task);
}

/* public api */
void construct_scheduler()
{
    int i;

    for(i = 0; i < ARRAY_NB(tasks); ++i)
        insert_free_task(&tasks[i]);
}

void schedule_task(uint32_t when, task_handler handler, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
    struct task *task = get_free_task();

    assert(task != NULL);
    task->when = when;
    task->handler = handler;
    task->p0 = p0;
    task->p1 = p1;
    task->p2 = p2;
    task->p3 = p3;

    if (when) {
        task->alarm.handler = schedule_later_handler;
        task->alarm.date_alarm = get_systick() + when;
        set_alarm(&task->alarm);
    } else {
        insert_runnable_task(task);
    }
}

void start_scheduler()
{
    while(1) {
        struct task *task;

        while ((task = get_runnable_task()) != NULL) {
            task->handler(task->p0, task->p1, task->p2, task->p3);
            insert_free_task(task);
        }

        wfi();
    }
}

#include <my_tasks.h>
#include <main.h>

TaskHandle_t tasks_2_ns_handle = NULL;

void task_setup(void)
{
    task_2ns_start();
}

void task_2ns_start(void)
{
    /* we create a new task here */
    xTaskCreatePinnedToCore(
        task_2_ns,          /* Task function. */
        "2ns Second Tasks", /* name of task. */
        5000,                /* Stack size of task */
        NULL,                 /* parameter of the task */
        1,                    /* priority of the task */
        &tasks_2_ns_handle, /* Task handle to keep track of created task */
        1);                   /* Task to be executed in Core-1 only */
}

void task_2_ns(void *parameter)
{
    TickType_t xLastWakeTime;

    const TickType_t xFrequency = (2 / portTICK_PERIOD_MS);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    /* loop forever */
    for (;;)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        _2ns_tasks();
    }
}
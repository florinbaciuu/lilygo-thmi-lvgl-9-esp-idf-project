
#include <stdbool.h>
#include <inttypes.h>
#include "esp_console.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_err.h"
#include <time.h>

#include "tasks_nfo.h"

static const char *TAG = "Tasks CMD";

#define TASK_MAX_COUNT 40
#define SECONDS_TO_MICROSECONDS(x) ((x) * 1000000)

const char *task_state[] = {"Running", "Ready", "Blocked", "Suspend", "Deleted", "Invalid"};

typedef struct
{
    uint32_t ulRunTimeCounter;
    uint32_t xTaskNumber;
} task_data_t;

static task_data_t previous_snapshot[TASK_MAX_COUNT];
static int task_top_id = 0;
static uint32_t total_runtime = 0;

task_data_t *getPreviousTaskData(uint32_t xTaskNumber)
{
    for (int i = 0; i < task_top_id; i++)
    {
        if (previous_snapshot[i].xTaskNumber == xTaskNumber)
        {
            return &previous_snapshot[i];
        }
    } // Try to find the task in the list of tasks
    assert(task_top_id < TASK_MAX_COUNT); // Allocate a new entry
    task_data_t *result = &previous_snapshot[task_top_id];
    result->xTaskNumber = xTaskNumber;
    task_top_id++;
    return result;
}

const char *getTimestamp()
{
    static char timestamp[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    return timestamp;
}

static int tasks_info(int argc, char **argv)
{
    ESP_LOGI(TAG, "-----------------Task Dump Start-----------------");
    printf("\n\r");
    uint32_t totalRunTime = 0;
    TaskStatus_t taskStats[TASK_MAX_COUNT] = {0};
    uint32_t taskCount = uxTaskGetSystemState(taskStats, TASK_MAX_COUNT, &totalRunTime);
    assert(task_top_id < TASK_MAX_COUNT);
    uint32_t totalDelta = totalRunTime - total_runtime;
    float f = 100.0 / totalDelta;
    printf("%.4s\t%.6s\t%.8s\t%.8s\t%.4s\t%-20s\n", "Load", "Stack", "State", "CoreID", "PRIO", "Name"); // Format headers in a more visually appealing way
    for (uint32_t i = 0; i < taskCount; i++)
    {
        TaskStatus_t *stats = &taskStats[i];
        task_data_t *previousTaskData = getPreviousTaskData(stats->xTaskNumber);
        uint32_t taskRunTime = stats->ulRunTimeCounter;
        float load = f * (taskRunTime - previousTaskData->ulRunTimeCounter);

        char formattedTaskName[19];                                                           // 16 caractere + 1 caracter pt terminatorul '\0' + 2 caractere pt paranteze"[]"
        snprintf(formattedTaskName, sizeof(formattedTaskName), "[%-16s]", stats->pcTaskName); // Format for the task's name with improved visibility
        char core_id_str[16];
        if (stats->xCoreID == -1 || stats->xCoreID == 2147483647)
        {
            snprintf(core_id_str, sizeof(core_id_str), "1/2");
        }
        else
        {
            snprintf(core_id_str, sizeof(core_id_str), "%d", stats->xCoreID);
        } // Customize how core ID is displayed for better clarity
        printf(
            "%.2f\t%" PRIu32 "\t%-4s\t%-4s\t%-4u\t%-19s\n", load, stats->usStackHighWaterMark, task_state[stats->eCurrentState], core_id_str, stats->uxBasePriority,
            formattedTaskName); // Print formatted output
        previousTaskData->ulRunTimeCounter = taskRunTime;
    }
    total_runtime = totalRunTime;
    printf("\n\r");
    ESP_LOGI(TAG, "-----------------Task Dump End-------------------");
    return 0;
}

static void register_tasks(void)
{
    const esp_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
}

/*
** Asta e functia principala de inregistrare a comenzilor
** pentru CLI-ul de versiune.
*/
void cli_register_tasks_info_command(void)
{
    register_tasks(); // Register the 'version' command
}
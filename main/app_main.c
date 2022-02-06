/*  Burglar alarm system master - BLE Server !
 *
 *  author : Thomas Georgiadis
 *
 * dat
 *
 *  @brief  This is the main for a master in a bluetooth network
 *          of 3 slaves and 1 master. Wifi is used in order to
 *          inform the owner when a breach has been detected.
 *          Bluetooth is used so that the slaves can inform the master
 *          that a breach has been detected. It is also used so that
 *          the master can arm/disarm the slaves.
 *          A push button is used in order to arm and disarm the alarm.
 *          A small siren is used so that it starts ringing when a breach
 *          has been detected.
 *
*/


#include "gpio_driver.h"
#include "timer_driver.h"
#include "ble_driver.h"


//Seconds
#define ARM_INTERVAL            10
#define DISARM_INTERVAL         20
#define MGNTC_REED_BREACH_ST    0
#define MGNTC_REED_CLOSED_ST    1

// Globals
static const char*    alarm_tag               = "BURGLAR_ALARM";
static uint8_t        psh_btn_state;
static uint8_t        psh_btn_last_state;
static uint8_t        mgn_reed_state;
static uint8_t        mgn_reed_last_state;
static uint8_t        bzr_state;
static uint8_t        timer_interval;

static uint8_t        alarm_arm_state;
static uint8_t        alarm_disarm_state;

static void timer_int_task(void* arg)
{
    // Timer initialization
    s_timer_queue = xQueueCreate(10, sizeof(example_timer_event_t));
    // should be called in door state change
    custom_timer_init(TIMER_GROUP_0, TIMER_0, true, timer_interval  );
    example_timer_event_t evt;
    for(;;)
    {
      if(xQueueReceive(s_timer_queue, &evt, portMAX_DELAY))
      {
          ESP_LOGI(alarm_tag,"Timer finished.");
          if(psh_btn_state == alarm_arm_state &&
             mgn_reed_state == 0)
          {
            bzr_state   = 1;
            gpio_set_level(BZR_PIN,bzr_state);
            ESP_LOGI(alarm_tag,"Buzzer activated.");
          }
          // stop the timer
          timer_pause(TIMER_GROUP_0, TIMER_0);
          vTaskDelete(NULL);
      }
    }
}

static void gpio_int_task(void* arg)
{
    uint32_t io_num;
    for(;;)
    {
      if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
      {
        if(io_num == PSH_BTN_PIN)
        {
          psh_btn_state = gpio_get_level(io_num);
          // Check for double events
          if(psh_btn_state != psh_btn_last_state)
          {
            ESP_LOGI(alarm_tag,"GPIO[%d] intr, val: %d\n", io_num, psh_btn_state);
            if (psh_btn_state == alarm_disarm_state)
            {
              // If the buzzer has been activated and disarm request was issued
              if(bzr_state == 1)
              {
                // stop the timer
                ESP_LOGI(alarm_tag,"Timer stopped.");
                bzr_state   = 0;
                gpio_set_level(BZR_PIN,bzr_state);
                ESP_LOGI(alarm_tag,"Buzzer deactivated.");
                timer_pause(TIMER_GROUP_0, TIMER_0);
              }
              else
              {
                timer_interval = ARM_INTERVAL;
                xTaskCreate(timer_int_task, "timer_int_task", 2048, NULL, 10, NULL);
              }
            }
          }
          else
          {
            // Double event detected.
          }
          psh_btn_last_state = psh_btn_state;
        }
        else if(io_num == MG_SNS_PIN)
        {
          mgn_reed_state = gpio_get_level(io_num);
          if(mgn_reed_state != mgn_reed_last_state)
          {
            // Check for a breach only when the alarm is armed.
            if(psh_btn_state == alarm_arm_state && bzr_state == 0)
            {
              timer_interval = DISARM_INTERVAL;
              xTaskCreate(timer_int_task, "timer_int_task", 2048, NULL, 10, NULL);

              ESP_LOGI(alarm_tag,
                       "Door state = %d on alarm arm state %d.\n",
                       gpio_get_level(MG_SNS_PIN),
                       alarm_arm_state);
            }
            else
            {
              ESP_LOGI(alarm_tag,
                       "Door state = %d on alarm disarm state %d.\n",
                       gpio_get_level(MG_SNS_PIN),
                       alarm_disarm_state);
            }
          }
          else
          {
            // Double event detected.
          }
          mgn_reed_last_state = mgn_reed_state;
        }
      }
    }
}

uint8_t get_alarm_current_state()
{

  if(psh_btn_state == alarm_arm_state)
  {
      return 1;
  }
  return 0;
}

void activate_buzzer(uint8_t door_state)
{
  ESP_LOGI(alarm_tag,"Activating Buzzer.");
  if(psh_btn_state == alarm_arm_state && door_state == MGNTC_REED_BREACH_ST)
  {
    bzr_state   = 1;
    gpio_set_level(BZR_PIN,bzr_state);
    ESP_LOGI(alarm_tag,"Buzzer activated.");
  }
  else if(door_state == MGNTC_REED_CLOSED_ST)
  {
    ESP_LOGI(alarm_tag,"Door closed.");
  }
  else
  {
    ESP_LOGI(alarm_tag,"Alarm is disarmed.");
  }
}

static void init(void)
{
  // Set the LOGS that you want to see.
  esp_log_level_set(alarm_tag,ESP_LOG_ERROR);

  // Initialize all gpios
  gpio_init();

  // Initialize states
  psh_btn_state          = gpio_get_level(PSH_BTN_PIN);
  psh_btn_last_state     = psh_btn_state;
  mgn_reed_state         = gpio_get_level(MG_SNS_PIN);
  mgn_reed_last_state    = mgn_reed_state;
  bzr_state              = gpio_get_level(BZR_PIN);

  alarm_disarm_state = psh_btn_state;
  if(alarm_disarm_state == 1)
  {
    alarm_arm_state = 0;
  }
  else
  {
    alarm_arm_state = 1;
  }

  ESP_LOGI(alarm_tag,"ARM state = %d\n", alarm_arm_state);
  ESP_LOGI(alarm_tag,"DISARM state = %d\n",alarm_disarm_state);

  // Initialize client and start searching for devices of interest
  ble_client_init();

}

void app_main(void)
{
    // initialize peripherals.
    init();

    //start gpio task
    xTaskCreate(gpio_int_task, "gpio_int_task", 2048, NULL, 10, NULL);
}


#include <zephyr/shell/shell.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include "common.h"
#include<math.h>
#include <zephyr/random/rand32.h>

LOG_MODULE_REGISTER(geryon_pwm, LOG_LEVEL_ERR);

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

int cpu_hz = 50E6;
int max_shift = 360;
int current_period = 10;
int current_pulse = 50;
int base_freq = 250E3;
int current_freq = 250000;
int current_shift = 120;
int current_jitter = 0;
int current_psc = 19;

void jitter_work_handler(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(jitter_work, jitter_work_handler);

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim_pwm) {
  if (htim_pwm->Instance == TIM1) {
    __HAL_RCC_TIM1_CLK_DISABLE();
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim_base) {
  if (htim_base->Instance == TIM2) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if (htim_base->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim_pwm) {
  if (htim_pwm->Instance == TIM1) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base) {
  if (htim_base->Instance == TIM2) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  } else if (htim_base->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (htim->Instance == TIM1) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  } else if (htim->Instance == TIM2) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  } else if (htim->Instance == TIM3) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

static void MX_TIM1_Init() {
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 19;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC3REF;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 5;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
  sConfigOC.Pulse = 3;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim1);
}

static void MX_TIM2_Init() {
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 19;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC2REF;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 5;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
  sConfigOC.Pulse = 3;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  __HAL_TIM_ENABLE_OCxPRELOAD(&htim2, TIM_CHANNEL_2);

  HAL_TIM_MspPostInit(&htim2);
}

static void MX_TIM3_Init() {
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 19;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_ITR1;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 5;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim3);
}

void pwm_init(void) {
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  k_msleep(100);

  htim1.Instance->CCR2 = 0;
  htim2.Instance->CCR1 = 0;
  htim3.Instance->CCR3 = 0;
}

void pwm_start(void) {
  htim1.Instance->CCR2 = 5;
  htim2.Instance->CCR1 = 5;
  htim3.Instance->CCR3 = 5;
}

void pwm_stop(void) {
  htim1.Instance->CCR2 = 0;
  htim2.Instance->CCR1 = 0;
  htim3.Instance->CCR3 = 0;
}

void jitter_work_handler(struct k_work *work) {
  if (current_jitter == 0) {
    return;
  }
  int offset_sync1 = sys_rand32_get() % current_jitter;
  int offset_sync2 = sys_rand32_get() % current_jitter;
  int offset_sync3 = sys_rand32_get() % current_jitter;

  int freq_sync1 = base_freq + base_freq * offset_sync1 / 100;
  int freq_sync2 = base_freq + base_freq * offset_sync2 / 100;
  int freq_sync3 = base_freq + base_freq * offset_sync3 / 100;

  float _temp = (float)(base_freq * (current_psc)) / (float)freq_sync1;
  int new_psc_1 = (int)(_temp + 0.5);
  _temp = (float)(base_freq * (current_psc)) / (float)freq_sync2;
  int new_psc_2 = (int)(_temp + 0.5);
  _temp = (float)(base_freq * (current_psc)) / (float)freq_sync3;
  int new_psc_3 = (int)(_temp + 0.5);

  LOG_DBG("Freq %d (%d) %d (%d) %d (%d)", freq_sync1, new_psc_1, freq_sync2,
          new_psc_2, freq_sync3, new_psc_3);

  htim1.Instance->PSC = new_psc_1;
  htim2.Instance->PSC = new_psc_2;
  htim3.Instance->PSC = new_psc_3;
  k_work_schedule(&jitter_work, K_SECONDS(10));
}

static int cmd_set_jitter(const struct shell *shell, size_t argc, char **argv) {
  int jitter = custom_atoi(argv[1]);
  if (jitter >= 10 || jitter <= -10) {
    shell_error(shell, "Invalid jitter parameter");
    return 0;
  }

  if (jitter == 0) {
    k_work_cancel_delayable(&jitter_work);
  } else {
    k_work_schedule(&jitter_work, K_NO_WAIT);
  }

  current_jitter = jitter;
  return 0;
}

static int cmd_set_pulse(const struct shell *shell, size_t argc, char **argv) {
  int input = custom_atoi(argv[1]);

  int pulse = input * (current_period) / 100;
  LOG_DBG("Pulse %d", pulse);
  htim1.Instance->CCR2 = pulse;
  htim2.Instance->CCR1 = pulse;
  htim3.Instance->CCR3 = pulse;
  current_pulse = input;
  return 0;
}

static int cmd_set_freq(const struct shell *shell, size_t argc, char **argv) {
  int new_freq = custom_atoi(argv[1]);
  float _temp = (float)(base_freq * (current_psc)) / (float)new_freq;
  int new_psc_1 = (int)(_temp + 0.5);
  _temp = (float)(base_freq * (current_psc)) / (float)new_freq;
  int new_psc_2 = (int)(_temp + 0.5);
  _temp = (float)(base_freq * (current_psc)) / (float)new_freq;
  int new_psc_3 = (int)(_temp + 0.5);
  htim1.Instance->PSC = new_psc_1;
  htim2.Instance->PSC = new_psc_2;
  htim3.Instance->PSC = new_psc_3;
  LOG_DBG("Freq %d (%d) %d (%d) %d (%d)", new_freq, new_psc_1, new_freq,
          new_psc_2, new_freq, new_psc_3);
  current_freq = new_freq;
  return 0;
}

static int cmd_set_start(const struct shell *shell, size_t argc, char **argv) {
  htim1.Instance->CCR2 = 5;
  htim2.Instance->CCR1 = 5;
  htim3.Instance->CCR3 = 5;
  return 0;
}

static int cmd_get_stop(const struct shell *shell, size_t argc, char **argv) {
  htim1.Instance->CCR2 = 0;
  htim2.Instance->CCR1 = 0;
  htim3.Instance->CCR3 = 0;
  return 0;
}

static int cmd_get_config(const struct shell *shell, size_t argc, char **argv) {
  shell_print(shell, "Frequency %d - Pulse %d - Jitter %d", current_freq,
              current_pulse, current_jitter);
  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_pwm,
    SHELL_CMD(config, NULL, "Get current configuration", cmd_get_config),
    SHELL_CMD(freq, NULL, "Set the frequency", cmd_set_freq),
    SHELL_CMD(jitter, NULL, "Set the jitter - noise", cmd_set_jitter),
    SHELL_CMD(pulse, NULL, "Set the pulse", cmd_set_pulse),
    SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_test,
    SHELL_CMD(start, NULL, "Start generating the PWM pulse ", cmd_set_start),
    SHELL_CMD(stop, NULL, "Stop generating the PWM pulse", cmd_get_stop),
    SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_cmd, SHELL_CMD(pwm, &sub_pwm, "Set/Get for PWM", NULL),
    SHELL_CMD(test, &sub_test, "Test command", NULL), SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(kama, &sub_cmd, "KamaCode shell interface", NULL);

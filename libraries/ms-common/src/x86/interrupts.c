/************************************************************************************************
 * @file   interrupts.c
 *
 * @brief  Interrupts Library Source file
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "interrupts.h"

/**
 * @brief   Defines the different states the x86 interrupt controller can be in
 * @details Masked mode means all interrupts are disabled
 *          Unmasked mode means all interrupts are enabled
 */
typedef enum {
  X86_INTERRUPT_STATE_NONE,     /**< Interrupt state none */
  X86_INTERRUPT_STATE_MASKED,   /**< Interrupt state masked */
  X86_INTERRUPT_STATE_UNMASKED, /**< Interrupt state unmasked */
} X86InterruptState;

/**
 * @brief   x86 Interrupt storage class
 * @details This mocks the hardware interrupts by storing a handler, masked bit and enabled bit
 */
typedef struct {
  bool enabled;                /**< Interrupt Channel enabled state */
  bool masked;                 /**< Interrupt Channel masked state */
  x86InterruptHandler handler; /**< Interrupt Channel interrupt handler */
} X86Interrupt;

/** @brief  Mock NVIC table that stores all function pointers */
static X86Interrupt s_nvic_handlers[NUM_STM32L433X_INTERRUPT_CHANNELS];

/** @brief  EXTI interrupt handler and state storage */
static X86Interrupt s_exti_interrupts[NUM_STM32L433X_EXTI_LINES];

/** @brief  Process ID handling the interrupts */
static pid_t s_pid = 0;

static void s_default_handler(uint8_t interrupt_id) {
  LOG_DEBUG("Default Handler\r\n");
}

static void s_nvic_handler(int signum, siginfo_t *info, void *context) {
  int interrupt_id = info->si_value.sival_int;

  if (interrupt_id >= 0 && (uint32_t)interrupt_id < NUM_STM32L433X_INTERRUPT_CHANNELS) {
    if ((s_nvic_handlers[interrupt_id].handler != NULL) && !s_nvic_handlers[interrupt_id].masked && s_nvic_handlers[interrupt_id].enabled) {
      s_nvic_handlers[interrupt_id].handler(interrupt_id);
    }
  }
}

static void s_exti_handler(int signum, siginfo_t *info, void *context) {
  int interrupt_id = info->si_value.sival_int;

  if (interrupt_id >= 0 && (uint32_t)interrupt_id < NUM_STM32L433X_EXTI_LINES) {
    if ((s_exti_interrupts[interrupt_id].handler != NULL) && !s_exti_interrupts[interrupt_id].masked && s_exti_interrupts[interrupt_id].enabled) {
      s_exti_interrupts[interrupt_id].handler(interrupt_id);
    }
  }
}

/* This must be completed in an interrupt handler due to the nature of user-space and kernel isolation */
/* The signal masks are only modifiable in the kernel space */
static void s_sig_state_handler(int signum, siginfo_t *info, void *ptr) {
  // We actually need to manipulate the block mask of the context that is
  // restored for this thread. So we alter |ctx| rather than directly calling
  // |pthread_sigmask| as this gets overridden on context switch after exiting
  // the handler.
  ucontext_t *ctx = ptr;

  /* Based on the sival_int change the state of interrupts. If invalid number silently ignore */
  if (info->si_value.sival_int == X86_INTERRUPT_STATE_MASKED) {
    sigaddset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
    sigaddset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
    sigaddset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  } else if (info->si_value.sival_int == X86_INTERRUPT_STATE_UNMASKED) {
    sigdelset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
    sigdelset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
    sigdelset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  }
}

void interrupt_init(void) {
  /* Assign the s_pid to be the process ID handling the interrupts */
  s_pid = getpid();

  /* Signal handler action */
  struct sigaction act;

  /* SA_RESTART allows syscalls to be retried */
  act.sa_flags = SA_SIGINFO | SA_RESTART;

  /* Emtpy blocking set since no signals are blocked to start */
  sigset_t block_mask;
  sigemptyset(&block_mask);

  /* Add a rule for low priority interrupts which blocks only other low priority signals */
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  act.sa_mask = block_mask;

  /* Update both the NVIC and EXTI signal handler */
  act.sa_sigaction = s_nvic_handler;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_LOW, &act, NULL);
  act.sa_sigaction = s_exti_handler;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_LOW, &act, NULL);

  /* Add a rule for normal priority interrupts which blocks low and other normal priority signals */
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  act.sa_mask = block_mask;

  /* Update both the NVIC and EXTI signal handler */
  act.sa_sigaction = s_nvic_handler;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_NORMAL, &act, NULL);
  act.sa_sigaction = s_exti_handler;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_NORMAL, &act, NULL);

  /* Add a rule for high priority interrupts which blocks all other interrupt signals */
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  act.sa_mask = block_mask;

  /* Update both the NVIC and EXTI signal handler */
  act.sa_sigaction = s_nvic_handler;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_HIGH, &act, NULL);
  act.sa_sigaction = s_exti_handler;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_HIGH, &act, NULL);

  /* Add a rule for the masking (critical sectioning) of the signals */
  act.sa_mask = block_mask;
  act.sa_sigaction = s_sig_state_handler;
  sigaction(SIGRTMIN + NUM_INTERRUPT_PRIORITIES, &act, NULL);

  /* Clear mock interrupt tables */

  for (uint8_t i = 0U; i < NUM_STM32L433X_INTERRUPT_CHANNELS; i++) {
    s_nvic_handlers[i].enabled = false;
    s_nvic_handlers[i].masked = true;
    s_nvic_handlers[i].handler = s_default_handler;
  }

  for (uint8_t i = 0U; i < NUM_STM32L433X_EXTI_LINES; i++) {
    s_exti_interrupts[i].enabled = false;
    s_exti_interrupts[i].masked = true;
    s_exti_interrupts[i].handler = s_default_handler;
  }
}

StatusCode interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  /* This function is for future expansion, in the situation we want to simulate NVIC interrupts */
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_nvic_register_handler(uint8_t irq_channel, x86InterruptHandler handler, const InterruptSettings *settings) {
  if (settings == NULL || settings->class >= NUM_INTERRUPT_CLASSES || settings->edge >= NUM_INTERRUPT_EDGES || irq_channel > NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (handler == NULL) {
    s_nvic_handlers[irq_channel].handler = s_default_handler;
  } else {
    s_nvic_handlers[irq_channel].handler = handler;
  }

  return STATUS_CODE_OK;
}

StatusCode interrupt_nvic_trigger(uint8_t irq_channel) {
  /* This function is for future expansion, in the situation we want to simulate NVIC interrupts */
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings) {
  if (settings == NULL || address == NULL || settings->class >= NUM_INTERRUPT_CLASSES || settings->edge >= NUM_INTERRUPT_EDGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_exti_interrupts[address->pin].enabled) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  s_exti_interrupts[address->pin].enabled = true;
  s_exti_interrupts[address->pin].masked = false;

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_register_handler(uint8_t line, x86InterruptHandler handler, const InterruptSettings *settings) {
  if (settings == NULL || settings->class >= NUM_INTERRUPT_CLASSES || settings->edge >= NUM_INTERRUPT_EDGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (handler == NULL) {
    s_exti_interrupts[line].handler = s_default_handler;
  } else {
    s_exti_interrupts[line].handler = handler;
  }

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_trigger(uint8_t line) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_clear_pending(uint8_t line) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_set_mask(uint8_t line, bool masked) {
  return STATUS_CODE_UNIMPLEMENTED;
}

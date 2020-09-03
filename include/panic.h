/*
 * panic.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_PANIC_H_
#define INCLUDE_PANIC_H_

#include "port_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum panic_code {
    PANIC_GENERAL_ERROR,
    PANIC_SOFT_REBOOT,
    PANIC_HARD_REBOOT,
    PANIC_ASSERT_FAIL,
    PANIC_EXPECT_FAIL,
    PANIC_SSP,                  /* stack smashing protector failure */
    PANIC_UNDEFINED
} panic_code_t;

void __attribute__((noreturn, noinline)) panic(panic_code_t crash_code, const char *message);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PANIC_H_ */

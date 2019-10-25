//
// Created by gta on 10/18/19.
//

#ifndef RTOS_TYPES_H
#define RTOS_TYPES_H

// #include <intrinsics.h>
// #include <stdint.h>
// #include <sys/stdint.h>

// Primitive type declarations
typedef   signed char      int8_t;   //  8-bit signed integer
typedef   signed int       int16_t;  // 16-bit signed integer
typedef   signed long      int32_t;  // 32-bit signed integer
typedef   signed long long int64_t;  // 64-bit signed integer

typedef unsigned char      uint8_t;  //  8-bit unsigned integer
typedef unsigned int       uint16_t; // 16-bit unsigned integer
typedef unsigned long      uint32_t; // 32-bit unsigned integer
typedef unsigned long long uint64_t; // 64-bit unsigned integer

typedef unsigned uint;

typedef uint32_t size_t;  // address size

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
typedef uint32_t word_t;  // register size
typedef uint32_t pc_t;    // program counter size
#else
typedef uint16_t word_t;  // register size
typedef uint16_t pc_t;    // program counter size
#endif

typedef int8_t bool;
#define true 1
#define false 0

#endif //RTOS_TYPES_H

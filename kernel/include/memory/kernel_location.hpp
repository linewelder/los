/**
 * @file
 * The values are contained in the variable's addresses.
 */

#pragma once

/**
 * Points to the beginning of the kernel in RAM.
 */
extern "C" int kernel_start;

/**
 * Points to the first 4KiB page after the kernel.
 */
extern "C" int kernel_end;

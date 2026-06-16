#pragma once
#ifndef ITERATIONS
#define ITERATIONS 1
#endif
// #define MSGSIZE 512
// #define MSGSIZE (2*1024) 
// #define MSGSIZE 4096
#define MSGSIZE (1<<14)
// #define ADLEN 1024
#define ADLEN 0
#if ADLEN > 0
#define AD_ARRSIZE ADLEN
#else 
#define AD_ARRSIZE 1
#endif

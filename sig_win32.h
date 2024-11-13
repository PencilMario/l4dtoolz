#ifndef SIG_WIN32_H
#define SIG_WIN32_H

#include "sigtool.h"

#define FUNC_T __stdcall
#define HOOK_DEF(ret, func, ...) static ret FUNC_T func(__VA_ARGS__)
#define CALL(ret, ptr, ...) ((ret (__thiscall *)(__VA_ARGS__))ptr)

#define rules_off    0x02
#define info_idx     0x88
#define sv_off       0x08
#define edict_idx    0x87
#define slots_idx    0x60
#define cookie_idx   0x5C
#define steam3_idx   0x40
#define steam3_off   0x0D
#define authrsp_idx  0x20
#define lobbyreq_idx 0x3D

#endif  // SIG_WIN32_H
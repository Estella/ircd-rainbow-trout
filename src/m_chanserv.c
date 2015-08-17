/*
 * IRC - src/m_chanserv.c
 * Is a channel service
 * Copyright [C] 2015- Janice "janicez" Johnson (janicez@umbrellix.tk)
 *
 * All rights reserved.
 *
 * Redistribution in both source and binary forms are permitted
 * provided that the above copyright notice remains unchanged.
 */


#include "struct.h"
#include "h.h"
#include "channel.h"
#include "numeric.h"
#include "sys.h"
#include "uthash.h"
// BIRCModule format (Bahamut)

void bircmodule_check(int modulever) {
  modulever = 1008;
}

void bircmodule_modinfo(char *modver, char *description) {
  modver = "1.0alpha1";
  description = "Provides channel services to a network running IRCd-rainbow-trout";
}

int bircmodule_init(void) {
}

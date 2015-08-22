/*
 * IRC - src/m_cap.c
 * SASL
 * Copyright [C] 2015- Janice "janicez" Johnson (janicez@umbrellix.tk)
 *
 * All rights reserved.
 *
 * Redistribution in both source and binary forms are permitted
 * provided that the above copyright notice remains unchanged.
 */

#define BIRCMODULE
#include "struct.h"
#include "common.h"
#include "hooks.h"
#include "h.h"
#include "channel.h"
#include "numeric.h"
#include "sys.h"
// BIRCModule format (Bahamut)

extern int do_user(char *, aClient *, aClient *, char *, char *, char *,
                   unsigned long, char *, char *, int (*cb)CCB);

static aClient *m_42serv_sptr = NULL;

#define CAP_SASL 0x0001
#define CAP_TLS  0x0002
#define IsCapable(sptr, x) ((sptr)->capabs & x)

static int m_cap(aClient *, aClient *, int, char **);
static int m_authen(aClient *, aClient *, int, char **);
void bircmodule_check(int modulever) {
  modulever = 1008;
}

void bircmodule_getinfo(char *modver, char *description) {
  modver = "1.0alpha1";
  description = "Provides SASL support to a network running IRCd-rainbow-trout";
}

int bircmodule_init(void *opaque) {
#ifndef USE_NEW_COMMAND_SYSTEM
  return 1; // Bail because we're not USE_NEW_COMMAND_SYSTEM enabled and thus not modular.
#endif
  msgstruct("AUTHEN", &m_authen, MAXPARA, 0, 0);
  return 0;
}

int bircmodule_globalcommand(void) {
  return 0;
}

int bircmodule_command(void) {
  return 0;
}

void bircmodule_shutdown(void) {
  // Kill the 42 command or whatever it was ;p
}

int gen_random() {

}

int m_cap(aClient *cptr, aClient *sptr, int parc, char *parv[]) {
  
  return 0;
}

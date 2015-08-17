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
#include "hooks.h"
#include "h.h"
#include "channel.h"
#include "numeric.h"
#include "sys.h"
// BIRCModule format (Bahamut)

static int m_test(aClient *, aClient *, int, char **);
void module_unload(void);

void bircmodule_check(int modulever) {
  modulever = 1008;
}

void bircmodule_modinfo(char *modver, char *description) {
  modver = "1.0alpha1";
  description = "Provides channel services to a network running IRCd-rainbow-trout";
}

int bircmodule_init(void *opaque) {
#ifndef USE_NEW_COMMAND_SYSTEM
  return 1; // Bail because we're not USE_NEW_COMMAND_SYSTEM enabled and thus not modular.
#endif
  msgstruct("42", m_test, MAXPARA, 0, 0);
  bircmodule_add_hook(MHOOK_UNLOAD, opaque, module_unload);
  return 0;
}

void module_unload(void) {
  // Kill the 42 command or whatever it was ;p
  delcommand("42");
}

static int m_test(aClient *cptr, aClient *sptr, int parc, char *parv[]) {
  if (MyClient(sptr))
    sendto_one(sptr, ":%s NOTICE %s :The answer to life the universe and everything is 42. - Modular command test complete.", me.name, sptr->name);
  else
    sendto_one(cptr, ":%s NOTICE %s :The answer to life the universe and everything is 42. - Remote modular command test complete.",me.name, sptr->name);
  return 0;
}

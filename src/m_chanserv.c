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
#include "common.h"
#include "hooks.h"
#include "h.h"
#include "channel.h"
#include "numeric.h"
#include "sys.h"
// BIRCModule format (Bahamut)

#define CBuf(x, l) char *x = (char *)malloc(sizeof(char)*l); memset(x, 0, l);

extern int do_user(char *, aClient *, aClient *, char *, char *, char *,
                   unsigned long, char *, char *, int (*cb)CCB);

static aClient *m_42serv_sptr = NULL;

static int m_42serv(aClient *, aClient *, char *, int, char **);
void bircmodule_check(int modulever) {
  modulever = 1008;
}

void bircmodule_getinfo(char *modver, char *description) {
  modver = "1.0alpha1";
  description = "Provides channel services to a network running IRCd-rainbow-trout";
}

int bircmodule_init(void *opaque) {
#ifndef USE_NEW_COMMAND_SYSTEM
  return 1; // Bail because we're not USE_NEW_COMMAND_SYSTEM enabled and thus not modular.
#endif
  char *res = (char *)MyMalloc(sizeof(char)*515);
  memset(res, 0, 515);
  m_42serv_sptr = make_client(NULL, &me);
  strncpyzt(m_42serv_sptr->name, "[42Serv]", 64);
  add_client_to_list(m_42serv_sptr);
  (void) add_to_client_hash_table("[42Serv]", m_42serv_sptr);
  do_user("[42Serv]", m_42serv_sptr, m_42serv_sptr, "fourtwo", "services.janicez.net", me.name, 0, "0.0.0.0", "IRCd pseudo user", &m_42serv);
  m_42serv_sptr->cb = &m_42serv;
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
  exit_client(m_42serv_sptr, m_42serv_sptr, m_42serv_sptr, "Module unloaded.");
}

int m_42serv CCB {
  int i = 0;
  char *res = (char *)MyMalloc(sizeof(char)*515);
  memset(res, 0, 515);
  sendto_gnotice ("To %s: :%s %s %s %s", me.name, parv[0], cmd, parv[1], parv[2]);
  // We have aClient *cptr, aClient *sptr, char *cmd, int parc, char *parv[]
  if (!strncasecmp("PING", cmd, 4)) {
    sprintf(res, "PONG :%s", parv[1]);
  } else if (!strncasecmp("PRIVMSG", cmd, 7)) {
    char *x  = (char *)MyMalloc(sizeof(char)*515); memset(x, 0, 515);
    if (NULL!=strchr(parv[2], ' ')) sscanf(parv[2], "%s ", x);
    else return 0;
    for (i = 0; i < strlen(x)+1; i++) *parv[2]++;
    if (!strncasecmp("echo", x, 4)) {
      sprintf(res, "NOTICE %s :As part of my duty, I am pleased to tell you that you typed: %s\r\n", sptr->name, parv[2]);
    } else if (!strncasecmp("help", x, 4)) {
      sprintf(res, "NOTICE %s :Yo. I'm %s, and my only command is ECHO - echos back what you type after it.\r\n", sptr->name, m_42serv_sptr->name);
    } else {
      sprintf(res, "NOTICE %s :I don't know what you mean by that. I'm %s, and my only command is ECHO - echos back what you type after it.\r\n", sptr->name, m_42serv_sptr->name);
    }
  }
  if (res[0] != '\0') {
    client_dopacket(m_42serv_sptr, res, strlen(res));
  }
  return 0;
}

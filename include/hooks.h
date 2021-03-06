/************************************************************************
 *   IRC - Internet Relay Chat, include/hooks.h
 *   Copyright (C) 2003 Lucas Madar
 *
 */

enum c_hooktype {
   CHOOK_10SEC,       /* Called every 10 seconds or so -- 
                       * not guaranteed to be 10 seconds *
                       * Params: None
                       * Returns void 
                       */
   CHOOK_HELD,        /* Called when client is held (requested modular password checking, or something)
                       * are done, acptr->ip is valid, 
                       * acptr->hostip is not "*"
                       * Params: 1: (aClient *) 
                       * Returns int
                       */
   CHOOK_PREACCESS,   /* Called before any access checks (dns, ident) 
                       * are done, acptr->ip is valid, 
                       * acptr->hostip is not "*"
                       * Params: 1: (aClient *) 
                       * Returns int
                       */
   CHOOK_POSTACCESS,  /* called after access checks are done 
                       * (right before client is put on network)
                       * Params: 1: (aClient *) 
                       * Returns int 
                       */
   CHOOK_POSTMOTD,    /* called after MOTD is shown to the client 
                       * Params: 1: (aClient *)
                       * Returns int 
                       */

   CHOOK_MSG,         /* called for every privmsg or notice
                       * Params: 3: (aClient *, int isnotice, char *msgtext), 
                       * Returns int 
                       */
   CHOOK_CHANMSG,     /* called for every privmsg or notice to a channel
                       * Params: 4: (aClient *source, aChannel *destination, 
                       *             int isnotice, char *msgtxt)
                       * Returns int
                       */
   CHOOK_USERMSG,     /* called for every privmsg or notice to a user
                       * Params: 4: (aClient *source, aClient *destination,
                       *             int isnotice, char *msgtxt)
                       * Returns int
                       */
   CHOOK_MYMSG,       /* called for every privmsg or notice to 'me.name' 
                       * Params: 3: (aClient *, int isnotice, char *msgtext)
                       * Returns int 
                       */
   CHOOK_JOIN,        /* called for local JOINs
                       * Params: 1: (aClient *, aChannel *)
                       * Returns int
                       */
   CHOOK_SENDBURST,   /* called from m_server.c during netbursts
                       * Params: 1: (aClient *)
                       * Returns void
                       */
   CHOOK_THROTTLE,    /* called from channel.c during throttle warnings
                       * Params: 3: (aClient *source, aChannel *channel,
                       *             int type, int jnum, int jtime)
                       * Returns void
                       */
   CHOOK_FORBID,      /* called from m_nick.c and channel.c when a user is
                       * attempting to use a forbidden nick or join a forbidden
                       * channel
                       * Params: 3: (aClient *source, char *name,
                       *             struct simBan *ban)
                       * Returns void
                       */
   CHOOK_SIGNOFF,     /* called on client exit (exit_client)
                       * Params: 1: (aClient *)
                       * Returns void */
   CHOOK_CHECKCANSPEAK,    // Used to implement extbans. Takes two parameters, (aClient *, aChannel *channel, char *text) returns int
   CHOOK_CHECKCANJOIN,    // Used to implement extbans. Takes two parameters, (aClient *, aChannel *channel) returns int
   CHOOK_DOINGWHOIS,     // After standard WHOIS lines, before EOW; takes four parameters (aClient *, aClient *, int parc, char **parv) same as WHOIS
   MHOOK_LOAD,        /* Called for modules loading and unloading */
   MHOOK_UNLOAD       /* Params: 2: (char *modulename, void *moduleopaque) */
};

extern int call_hooks(enum c_hooktype hooktype, ...);
extern int init_modules();

#define MODULE_INTERFACE_VERSION 1008 /* the interface version (hooks, modules.c commands, etc) */

#ifdef BIRCMODULE
extern void *bircmodule_add_hook(enum c_hooktype, void *, void *);
extern void bircmodule_del_hook();
extern int bircmodule_malloc(int);
extern void bircmodule_free(void *);
#endif

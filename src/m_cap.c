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

// Base64.c from OrangeTide libcgi {
/* base64.c : base-64 / MIME encode/decode */
/* PUBLIC DOMAIN - Jon Mayo - November 13, 2003 */
/* $Id: base64.c 156 2007-07-12 23:29:10Z orange $ */

static const uint8_t base64enc_tab[64]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t base64dec_tab[256]= {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
	 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
	255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

void base64encode(const unsigned char in[3], unsigned char out[4], int count)
{
	out[0]=base64enc_tab[(in[0]>>2)];
	out[1]=base64enc_tab[((in[0]&3)<<4)|(in[1]>>4)];
	out[2]=count<2 ? '=' : base64enc_tab[((in[1]&15)<<2)|(in[2]>>6)];
	out[3]=count<3 ? '=' : base64enc_tab[(in[2]&63)];
}

int base64decode(const char in[4], char out[3])
{
	uint8_t v[4];

	v[0]=base64dec_tab[(unsigned)in[0]];
	v[1]=base64dec_tab[(unsigned)in[1]];
	v[2]=base64dec_tab[(unsigned)in[2]];
	v[3]=base64dec_tab[(unsigned)in[3]];

	out[0]=(v[0]<<2)|(v[1]>>4); 
	out[1]=(v[1]<<4)|(v[2]>>2); 
	out[2]=(v[2]<<6)|(v[3]); 
	return (v[0]|v[1]|v[2]|v[3])!=255 ? in[3]=='=' ? in[2]=='=' ? 1 : 2 : 3 : 0;
}

/* decode a base64 string in one shot */
int base64_decode(size_t in_len, const char *in, size_t out_len, unsigned char *out) {
	unsigned ii, io;
	uint_least32_t v;
	unsigned rem;

	for(io=0,ii=0,v=0,rem=0;ii<in_len;ii++) {
		unsigned char ch;
		if(isspace(in[ii])) continue;
		if(in[ii]=='=') break; /* stop at = */
		ch=base64dec_tab[(unsigned)in[ii]];
		if(ch==255) break; /* stop at a parse error */
		v=(v<<6)|ch;
		rem+=6;
		if(rem>=8) {
			rem-=8;
			if(io>=out_len) return -1; /* truncation is failure */
			out[io++]=(v>>rem)&255;
		}
	}
	if(rem>=8) {
		rem-=8;
		if(io>=out_len) return -1; /* truncation is failure */
		out[io++]=(v>>rem)&255;
	}
	return io;
}

int base64_encode(size_t in_len, const unsigned char *in, size_t out_len, char *out) {
	unsigned ii, io;
	uint_least32_t v;
	unsigned rem;

	for(io=0,ii=0,v=0,rem=0;ii<in_len;ii++) {
		unsigned char ch;
		ch=in[ii];
		v=(v<<8)|ch;
		rem+=8;
		while(rem>=6) {
			rem-=6;
			if(io>=out_len) return -1; /* truncation is failure */
			out[io++]=base64enc_tab[(v>>rem)&63];
		}
	}
	if(rem) {
		v<<=(6-rem);
		if(io>=out_len) return -1; /* truncation is failure */
		out[io++]=base64enc_tab[v&63];
	}
	while(io&3) {
		if(io>=out_len) return -1; /* truncation is failure */
		out[io++]='=';
	}
	if(io>=out_len) return -1; /* no room for null terminator */
	out[io]=0;
	return io;
}

/* vim: ts=4 sts=0 noet sw=4
*/

// }

static char chartab[37] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static char numtab[11] = "0123456789";
static int newrand = 0;
static FILE *randptr = NULL;

static int rand_digit() {
  if (newrand) {
    return (int)(fgetc(randptr) + fgetc(randptr) + fgetc(randptr) + fgetc(randptr) + fgetc(randptr));
  } else {
    return rand() % (254<RAND_MAX)?254:RAND_MAX;
  }
}

static char *
gen_uid()
{
    char *hazh = malloc(15);
    memset(hazh, 0, 15);
    int i;
    for (i = 0; i < 7; i++) {
      hazh[i] = chartab[rand_digit()%35];
    }
    return hazh;
}


extern int do_user(char *, aClient *, aClient *, char *, char *, char *,
                   unsigned long, char *, char *, int (*cb)CCB);

#define CAP_SASL 0x0001
#define CAP_TLS  0x0002
#define IsCapable(sptr, x) ((sptr)->capabs & x)
#define SetCapable(sptr, x) ((sptr)->capabs |= x)

int m_cap(aClient *, aClient *, int, char **);
int m_authen(aClient *, aClient *, int, char **);
int m_ac(aClient *, aClient *, int, char **);
int mr_authen(aClient *, aClient *, int, char **);
int cap_doing_whois(aClient *, aClient *, aClient *, int, char **);
#ifndef USE_NEW_COMMAND_SYSTEM
#error "You are using a version of ircd-rainbow-trout that does not have USE_NEW_COMMAND_SYSTEM defined near the bottom of include/setup.h."
#error "Please add #define USE_NEW_COMMAND_SYSTEM to setup.h and recompile IRCd."
#endif
struct aSasl {
  aClient *key;
  char *puid;
  char *uname;
  aClient *agent;
  UT_hash_handle hh;
  UT_hash_handle bhh;
};

struct aSasl *sasl_table = NULL;
struct aSasl *sasl_backtable = NULL;

int add_sasl_user (aClient *user, char *puid) {
  struct aSasl *sasl = malloc(sizeof(struct aSasl));
  sasl->key = user;
  sasl->puid = puid;
  sasl->agent = NULL;
  sasl->uname = NULL;
  HASH_ADD_KEYPTR(hh, sasl_table, sasl->key, sizeof(aClient), sasl);
  HASH_ADD_KEYPTR(hh, sasl_backtable, sasl->puid, strlen(sasl->puid), sasl);
  user->capabs |= CAP_SASL;
  return 0;
}

char *get_sasl_puid (aClient *user) {
  struct aSasl *u = NULL;
  HASH_FIND_PTR(sasl_table, user, u);
  if (u == NULL) return NULL;
  return u->puid;
}

aClient *get_sasl_agent (aClient *user) {
  struct aSasl *u = NULL;
  if (user == NULL) /* Bail. */ return NULL;
  HASH_FIND_PTR(sasl_table, user, u);
  if (u == NULL) return NULL;
  return u->agent;
}

void set_sasl_agent (aClient *user, aClient *agent) {
  struct aSasl *u = NULL;
  HASH_FIND_PTR(sasl_table, user, u);
  if (u == NULL) return;
  u->agent = agent;
}

void del_sasl_user(aClient *user) {
  struct aSasl *u = NULL;
  HASH_FIND_PTR(sasl_table, user, u);
  if (u == NULL) return;
  HASH_DEL(sasl_backtable, u);
  HASH_DEL(sasl_table, u);
}

aClient *get_sasl_user (char *puid) {
  struct aSasl *u = NULL;
  HASH_FIND_STR(sasl_backtable, puid, u);
  if (u == NULL) return NULL;
  return u->key;
}

void bircmodule_check(int *modulever) {
  *modulever = 1008;
}

void bircmodule_getinfo(char **modver, char **description) {
  *modver = "1.0alpha1";
  *description = "Provides SASL support to a network running IRCd-rainbow-trout";
}

int bircmodule_globalcommand(void) {
  return 0;
}

int bircmodule_command(void) {
  return 0;
}

void bircmodule_shutdown(void) {
  // Kill the 42 command or whatever it was ;p
  delcommand("AC");
  delcommand("ACCOUNT");
  delcommand("AUTHEN");
  delcommand("AUTHENTICATE");
  delcommand("CAP");
  delcommand("STARTTLS");
  if (randptr) fclose(randptr);
}

int m_cap(aClient *cptr, aClient *sptr, int parc, char *parv[]) {
  extern int ssl_capable;
  char *iscap = malloc(sizeof(char)*450);
  int i = 0;
  memset(iscap, 0, 450);
  if (IsServer(cptr)) {
    sendto_gnotice("Warning: server %s is mixing client and server protocol (CAP)", cptr->name);
    sendto_serv_butone(&me, ":%s GNOTICE :Warning: server %s is mixing client and server protocol (CAP)", me.name, cptr->name);
    return 0;
  }
  if (cptr != sptr) return 0;
  if (!IsRegistered(cptr)) SetOnHold(cptr);
  if (!strcasecmp(parv[1], "REQ")) { for (i = 2; i < parc; i++)
    {
      if (!strcasecmp(parv[i], "SASL"))
        SetCapable(cptr, CAP_SASL);
      if (!strcasecmp(parv[i], "TLS") && ssl_capable)
        SetCapable(cptr, CAP_TLS);
    }
    if (IsCapable(cptr, CAP_SASL)) strncat(iscap, "sasl ", 450 - 5);
    if (IsCapable(cptr, CAP_TLS)) strncat(iscap, "tls ", 445 - 4);
    sendto_one(&me, cptr, "CAP ACK :%s", iscap);
  }
  if (!strcasecmp(parv[1], "END")) {
    UnsetOnHold(cptr);
    if (cptr->name[0]) register_user(cptr, sptr, cptr->name, cptr->username, cipntoa(cptr));
  }
  if (!strcasecmp(parv[1], "LS")) sendto_one(&me, cptr, "CAP LS :%s %s", "sasl", "tls");
  return 0;
}

int mr_authen(aClient *cptr, aClient *sptr, int parc, char *parv[]) {
  if (IsServer(cptr)) {
    sendto_gnotice("Warning: server %s is mixing client and server protocol (AUTHENTICATE, servers should use AUTHEN to handle SASL)", cptr->name);
    sendto_serv_butone(&me, ":%s GNOTICE :Warning: server %s is mixing client and server protocol (AUTHENTICATE, servers should use AUTHEN to handle SASL)", me.name, cptr->name);
    return 0;
  }
  if (IsRegistered(cptr))
    return 0;

  char *uid = get_sasl_puid(cptr);
  if (!IsCapable(cptr, CAP_SASL)) return 0;
  if (strlen(parv[1]) > 400) {
    sendto_one(&me, sptr, ":%s 905 %s :ERR_SASLTOOLONG SASL message too long");
  } else {
    if (*uid == '\0' && get_sasl_agent(cptr) == NULL) {
      uid = gen_uid();
      sendto_capab_serv_butone(&me, CAPAB_ESVID, 0, ":%s AUTHEN %s %s %s S %s", me.name, me.name, "*", uid, parv[1]);
      add_sasl_user(cptr, uid);
    } else {
      aClient *saslagent = get_sasl_agent(cptr);
      sendto_capab_serv_butone(&me, CAPAB_ESVID, 0, ":%s AUTHEN %s %s %s S %s", me.name, me.name, (saslagent == NULL)?"*":saslagent->name, uid, parv[1]);
    }
  }
  return 0;
}

char *b64pencode(aClient *targ) {
  char *tb64 = MyMalloc(    ((strlen(targ->passwd)*4)/3)+6  );
  char *pw = targ->passwd;
  char *npw = MyMalloc(strlen(targ->passwd)+2);
  int i = 0, j = 0;
  memset(tb64, 0, ((strlen(targ->passwd)*4)/3)+6 );
  memset(npw, 0, strlen(targ->passwd)+2);
  for (i=0; pw[i]; i++) {
    if (pw[i] == '/') {
      j++;
      if (j == 3) break;
      npw[i] = '\0';
    }
    else npw[i] = pw[i];
  }
  base64_encode(strlen(targ->passwd)+1, npw, ((strlen(targ->passwd)*4)/3)+5, tb64);
  return tb64;
}

int m_authen(aClient *cptr, aClient *sptr, int parc, char *parv[]) {
  if (!IsServer(sptr)) return 0;
  if (!strcasecmp(parv[2], me.name)) {
    aClient *targ = get_sasl_user(parv[3]);
    aClient *agent = get_sasl_agent(targ);

    /*
     * error, error, error! If targ is null by now, we're in a right loada trouble.
     */
    if (targ == NULL) {
      sendto_capab_serv_butone(&me, CAPAB_ESVID, 0, ":%s GNOTICE :A SASL authentication failed due to null targ.", me.name);
      return;
    }
    switch (*parv[4]) {
     case 'C':
       if (agent != NULL) if (agent != sptr) break;
       if (agent == NULL) set_sasl_agent(targ, sptr);
       // Have to be silent for LOC users, hence the IsCapable check
       if (IsCapable(targ, CAP_SASL)) sendto_one(&me, targ, "AUTHENTICATE %s", parv[4]);
       break;
     case 'L':
       strncpyzt(targ->account, parv[5], NICKLEN + 1);
       break;
     case 'D':
       UnsetOnHold(targ);
       switch (*parv[5]) {
         case 'F':
           if (IsCapable(targ, CAP_SASL))
             sendto_one(&me, targ, ":%s 904 %s :AUTHFAIL SASL authentication failed", me.name, targ->name);
           else
             sendto_one(&me, targ, ":%s NOTICE * :Authentication failed.", me.name);
           break;
         case 'S':
           if (IsCapable(targ, CAP_SASL))
             sendto_one(&me, targ, ":%s 903 %s :AUTHSUCCESS SASL authentication successful", me.name, targ->name);
           sendto_one(&me, targ, ":%s NOTICE * :Authentication succeeded as %s.", me.name, targ->account);
           break;
       }
       if (!IsRegistered(cptr) && sptr->name[0] && sptr->info[0]) do_user(sptr->name, cptr, sptr, sptr->username, cipntoa(sptr), me.name, 0, 0, sptr->info, NULL);
       return 0;
    }
  }
  sendto_capab_serv_butone(cptr, CAPAB_ESVID, 0, ":%s AUTHEN %s %s %s %c %s%s%s",
             sptr->name, parv[1], parv[2], parv[3], *parv[4], parv[5], parc > 6 ? " " : "" ,parc > 6 ? parv[6] : "");
  return 0;
}

int cap_doing_whois(aClient *cptr, aClient *sptr, aClient *tptr, int parc, char *parv[]) {
  if (tptr->account[0] == '\0') return 0;
  sendto_one(sptr, sptr, ":%s 330 %s %s %s :ACCOUNT", me.name, parv[0], tptr->name, tptr->account);
  return 0;
}

int m_ac(aClient *cptr, aClient *sptr, int parc, char *parv[]) {
  if (!IsServer(cptr)) return 0;
  aClient *acptr;
  char oldhost[HOSTLEN+1];

  if(!IsULine(sptr) || parc<2)
      return 0; /* Not a u:lined server or not enough parameters */

  if(!(acptr = find_person(parv[1], NULL)))
      return 0; /* Target user doesn't exist */

  strncpyzt(acptr->account, parv[2], NICKLEN + 1);

  sendto_capab_serv_butone(cptr, CAPAB_ESVID, 0, ":%s AC %s%s%s", parv[0], parv[1], BadPtr(parv[2])?"":" ", BadPtr(parv[2])?acptr->account:"");

  return 0;
}

int held_authen(aClient *cptr) {
  char *uid;

  uid = gen_uid();
  add_sasl_user(cptr, uid);

  sendto_capab_serv_butone(&me, CAPAB_ESVID, 0, ":%s AUTHEN %s %s %s S %s %s", me.name, me.name, "*", uid, "LOC", cptr->passwd);

  return 0;
}

int bircmodule_init(void *opaque) {
  FILE *fp;
  msgstruct("AC", &m_ac, MAXPARA, 0, 0);
  msgstruct("ACCOUNT", &m_ac, MAXPARA, 0, 0);
  msgstruct("AUTHEN", &m_authen, MAXPARA, 0, 0);
  msgstruct("AUTHENTICATE", &mr_authen, MAXPARA, MF_UNREG, 0);
  msgstruct("CAP", &m_cap, MAXPARA, MF_UNREG, 0);
  //msgstruct("STARTTLS", &m_starttls, MAXPARA, MF_UNREG, 0);
  srand(time(NULL));
  if ((randptr = fopen("/dev/urandom", "r")) != NULL) {
    newrand = 1;
  }
  if (isatty(0)) {
    printf("me.name:%s %s %s %s\n", me.name, gen_uid(), gen_uid(), gen_uid()); fflush(stdout);
    printf("%s %s %s\n", gen_uid(), gen_uid(), gen_uid()); fflush(stdout);
    printf("%s %s %s\n", gen_uid(), gen_uid(), gen_uid()); fflush(stdout);
  }
  bircmodule_add_hook(CHOOK_DOINGWHOIS, opaque, &cap_doing_whois);
  bircmodule_add_hook(CHOOK_HELD, opaque, &held_authen);
  return 0;
}


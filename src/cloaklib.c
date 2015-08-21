/*
 * taken from Charybdis: an advanced ircd
 * ip_cloaking.c: provide user hostname cloaking
 *
 * Written originally by nenolod, altered to use FNV by Elizabeth in 2008
 * altered some more by groente
 * altered yet more by janicez for ircd-rainbow-trout.
 */

#define BIRCMODULE // to stop hook addition complaints
#include "struct.h"
#include "common.h"
#include "hooks.h"
#include "h.h"
#include "channel.h"
#include "numeric.h"
#include "sys.h"
#include "dh.h"
#include "arpa/inet.h"

static void *hookopaque;

static char chartab[37] = {
 'a', 'b', 'c', 'd', 'e', 'f', 'g',
 'h', 'i', 'j', 'k', 'l', 'm', 'n',
 'o', 'p', 'q', 'r', 's', 't', 'u',
 'v', 'w', 'x', 'y', 'z',
 '1', '2', '3', '4', '5',
 '6', '7', '8', '9', '0',
 0
};

static char *
do_ip_cloak_part(const char *part)
{
    char *hash, *hazh = malloc(strlen(part)+1+3), *hatch = malloc(strlen(part)+1+3);
    memset(hatch, 0, strlen(part)+1+3);
    memset(hazh, 0, strlen(part)+1+3);
    if (CloakKey[0])
    { char *key = malloc(strlen(part)+1+sizeof(CloakKey));
      strncpy(key, part, strlen(part)+1+sizeof(CloakKey));
      strncat(key, CloakKey, strlen(part)+1+sizeof(CloakKey));
    } else {
      char *key = malloc(strlen(part)+1);
      strncpy(key, part, strlen(part)+1);
    }
    int i;
    void *rc4state = rc4_initstate((unsigned char *)strdup(part), strlen(part));
    hash = malloc(strlen(part)+1+3);
    rc4_process_stream_to_buf(rc4state, part, hash, strlen(part));
    for (i = 0; i < strlen(part); i++) {
      hazh[i] = chartab[hash[i]%35];
    }
    rc4_destroystate(rc4state);
    return hazh;
}

static void
do_ip_cloak(const char *inbuf, char *outbuf)
{
    unsigned int a, b, c, d;
    struct in_addr in_addr;
    char buf[512], *alpha, *beta, *gamma;
    alpha = malloc(512);
    beta = malloc(512);
    gamma = malloc(512);
    inet_pton(AF_INET, inbuf, &in_addr);
    a = (in_addr.s_addr & 0xff000000) >> 24;
    b = (in_addr.s_addr & 0x00ff0000) >> 16;
    c = (in_addr.s_addr & 0x0000ff00) >> 8;
    d = in_addr.s_addr & 0x000000ff;
    snprintf(alpha, 511, "%s", inbuf);
    snprintf(beta, 511, "%u.%u.%u", a, b, c);
    snprintf(gamma, 511, "%u.%u", a, b);
    ircsprintf(outbuf, "%s.%s.%s.i4msk", do_ip_cloak_part(alpha), do_ip_cloak_part(beta), do_ip_cloak_part(gamma));
}

static void
do_host_cloak_ipv6(const char *inbuf, char *outbuf)
{
    char *a, *b, *c, *d;
    char buf[512], *alpha, *beta, *gamma;
    struct in6_addr in_addr;
    a = malloc(sizeof(char)*512);
    b = malloc(sizeof(char)*512);
    c = malloc(sizeof(char)*512);
    alpha = malloc(sizeof(char)*512);
    beta = malloc(sizeof(char)*512);
    gamma = malloc(sizeof(char)*512);
    inet_pton(AF_INET6, inbuf, &in_addr);
    ircsprintf(c, "%2x%2x.%2x%2x.%2x%2x.%2x%2x.%2x%2x.%2x%2x.%2x%2x.%2x%2x",
		in_addr.s6_addr[0],
		in_addr.s6_addr[1],
		in_addr.s6_addr[2],
		in_addr.s6_addr[3],
		in_addr.s6_addr[4],
		in_addr.s6_addr[5],
		in_addr.s6_addr[6],
		in_addr.s6_addr[7],
		in_addr.s6_addr[8],
		in_addr.s6_addr[9],
		in_addr.s6_addr[10],
		in_addr.s6_addr[11],
		in_addr.s6_addr[12],
		in_addr.s6_addr[13],
		in_addr.s6_addr[14],
		in_addr.s6_addr[15]
	);
    ircsprintf(b, "%2x%2x.%2x%2x.%2x%2x.%2x%2x",
		in_addr.s6_addr[0],
		in_addr.s6_addr[1],
		in_addr.s6_addr[2],
		in_addr.s6_addr[3],
		in_addr.s6_addr[4],
		in_addr.s6_addr[5],
		in_addr.s6_addr[6],
		in_addr.s6_addr[7]
	);
    ircsprintf(a, "%2x%2x.%2x%2x",
		in_addr.s6_addr[0],
		in_addr.s6_addr[1],
		in_addr.s6_addr[2],
		in_addr.s6_addr[3]
	);
    ircsprintf(alpha, "%s", inbuf);
    ircsprintf(beta, "%s.%s.%s", a, b, c);
    ircsprintf(gamma, "%s.%s", a, b);
    ircsprintf(outbuf, "%s:%s:%s:i6msk", do_ip_cloak_part(alpha), do_ip_cloak_part(beta), do_ip_cloak_part(gamma));
}

static void
do_host_cloak_host(const char *inbuf, char *outbuf)
{
    unsigned char *hash;
    char buf[3];
    char output[HOSTLEN+1];
    int i, j;

    output[0]=0;

    char *oldhost;
    j = 0;
    oldhost = strdup(inbuf);
    int hostlen = 0;

    for (i = 0; i < strlen(oldhost); i++) {
        oldhost++;
        hostlen++;
        if (*oldhost == '.') {
            break;
        }
    }

    char *nhost = strndup(oldhost, hostlen);

    strncpy(outbuf,Network_Name,HOSTLEN+1);
    strncat(outbuf,"-",HOSTLEN-strlen(outbuf));
    strncat(outbuf,do_ip_cloak_part(nhost),HOSTLEN-strlen(outbuf));

    strncat(outbuf,output,HOSTLEN-strlen(outbuf));
    strncat(outbuf,oldhost,HOSTLEN-strlen(outbuf));
}

static void
do_host_cloak_ip(const char *inbuf, char *outbuf)
{
    /* None of the characters in this table can be valid in an IP */
    char *tptr;
    int totalcount = 0;
    int ipv6 = 0;

    if (strchr(inbuf, ':')) {
        ipv6 = 1;

        /* Damn you IPv6...
         * We count the number of colons so we can calculate how much
         * of the host to cloak. This is because some hostmasks may not
         * have as many octets as we'd like.
         *
         * We have to do this ahead of time because doing this during
         * the actual cloaking would get ugly
         */
        for (tptr = inbuf; *tptr != '\0'; tptr++)
            if (*tptr == ':')
                totalcount++;
    } else if (!strchr(inbuf, '.'))
        return;
    if (ipv6)
       do_host_cloak_ipv6(inbuf, outbuf);
    else
       do_ip_cloak(inbuf, outbuf);
}

int do_make_cloak (aClient *sptr) {
  char *in = sptr->user->host;
  if (*in && !sptr->user->host[0]) strcpy(sptr->user->realhost, in);
  char *out = sptr->user->mangledhost;
  if (!myncmp(in, sptr->hostip, strlen(in)))
    do_host_cloak_ip(in, out);
  else
    do_host_cloak_host(in, out);
  if (isatty(0)) {
    printf("User cloaked from host %s[%s] to %s\n", in, sptr->hostip, out);
    fflush(stdout);
  }
  return 0;
}

int bircmodule_init(void *opaque) {
  hookopaque = bircmodule_add_hook(CHOOK_POSTACCESS, opaque, &do_make_cloak);
  return 0;
}

void bircmodule_check(int modulever) {
  modulever = 1008;
}

void bircmodule_getinfo(char **modver, char **description) {
  *modver = "1.0alpha1";
  *description = "Provides RC4-based cloaking to a network running IRCd-rainbow-trout";
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


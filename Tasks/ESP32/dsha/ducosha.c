// duco_sha1_range.c  (ELF dla ESP32) — soft SHA-1 + częsty yield - bez hw SHA, bo przytyka nam cache i psuje WIFI
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PAYLOAD_PATH "/spiffs/payload"
#define SHA1_LEN     20
#define SHA1_HEX_LEN (SHA1_LEN * 2)

/* --------- minimalny soft SHA-1 (public domain) --------- */
typedef struct { uint32_t s[5]; uint64_t n; uint8_t b[64]; } sha1_ctx;

static uint32_t rol(uint32_t x, int n){ return (x<<n)|(x>>(32-n)); }
static void sha1_init(sha1_ctx* c){
  c->s[0]=0x67452301u; c->s[1]=0xEFCDAB89u; c->s[2]=0x98BADCFEu; c->s[3]=0x10325476u; c->s[4]=0xC3D2E1F0u; c->n=0;
}
static void sha1_blk(sha1_ctx* c, const uint8_t *p){
  uint32_t w[80];
  for (int i=0;i<16;i++) w[i]=(p[4*i]<<24)|(p[4*i+1]<<16)|(p[4*i+2]<<8)|p[4*i+3];
  for (int i=16;i<80;i++) w[i]=rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
  uint32_t a=c->s[0],b=c->s[1],cc=c->s[2],d=c->s[3],e=c->s[4];
  for (int i=0;i<80;i++){
    uint32_t f,k;
    if (i<20){ f=(b&cc)|((~b)&d); k=0x5A827999u; }
    else if (i<40){ f=b^cc^d; k=0x6ED9EBA1u; }
    else if (i<60){ f=(b&cc)|(b&d)|(cc&d); k=0x8F1BBCDCu; }
    else { f=b^cc^d; k=0xCA62C1D6u; }
    uint32_t t=rol(a,5)+f+e+k+w[i];
    e=d; d=cc; cc=rol(b,30); b=a; a=t;
  }
  c->s[0]+=a; c->s[1]+=b; c->s[2]+=cc; c->s[3]+=d; c->s[4]+=e;
}
static void sha1_upd(sha1_ctx* c, const uint8_t* data, size_t len){
  size_t i=(size_t)(c->n & 63u); c->n+=len;
  if (i){ size_t t=64-i; if (len<t){ memcpy(c->b+i,data,len); return; } memcpy(c->b+i,data,t); sha1_blk(c,c->b); data+=t; len-=t; }
  while (len>=64){ sha1_blk(c,data); data+=64; len-=64; }
  if (len) memcpy(c->b,data,len);
}
static void sha1_fin(sha1_ctx* c, uint8_t out[20]){
  size_t i=(size_t)(c->n & 63u);
  c->b[i++]=0x80; if (i>56){ while(i<64) c->b[i++]=0; sha1_blk(c,c->b); i=0; }
  while(i<56) c->b[i++]=0;
  uint64_t bits = c->n*8;
  for (int j=7;j>=0;--j) c->b[56+(7-j)] = (uint8_t)((bits >> (8*j)) & 0xFF);
  sha1_blk(c,c->b);
  for (int j=0;j<5;j++){ out[4*j+0]=(uint8_t)(c->s[j]>>24); out[4*j+1]=(uint8_t)(c->s[j]>>16); out[4*j+2]=(uint8_t)(c->s[j]>>8); out[4*j+3]=(uint8_t)(c->s[j]); }
}
/* -------------------------------------------------------- */

/* --- drobne utilsy bez ctype/memcmp/strcmp --- */
static size_t my_strlen(const char *s){ const char *p=s; while(*p) ++p; return (size_t)(p-s); }
static void   my_memcpy(char *d, const char *s, size_t n){ while(n--) *d++ = *s++; }
static int    hex_nibble(char c){
  if (c>='0' && c<='9') return c-'0';
  if (c>='a' && c<='f') return 10 + (c-'a');
  if (c>='A' && c<='F') return 10 + (c-'A');
  return -1;
}
static int hex_to_bytes20_exact(const char *hex, uint8_t out[SHA1_LEN]){
  for (int i=0;i<SHA1_LEN;i++){
    int hi=hex_nibble(hex[i*2]); int lo=hex_nibble(hex[i*2+1]);
    if (hi<0 || lo<0) return 0;
    out[i] = (uint8_t)((hi<<4)|lo);
  }
  return 1;
}
static int digest_eq_20(const uint8_t *a, const uint8_t *b){
  for (int i=0;i<20;i++) if (a[i]!=b[i]) return 0;
  return 1;
}
// szybkie u32 -> ASCII (zwraca liczbę cyfr)
static int u32_to_dec(char *dst, unsigned int v){
  char tmp[10]; int n=0;
  do { tmp[n++] = (char)('0' + (v%10)); v/=10; } while(v);
  for (int i=n-1;i>=0;--i) *dst++ = tmp[i];
  return n;
}

int local_main(void)
{
  FILE *f = fopen(PAYLOAD_PATH, "r");
  if (!f) { return 1; }

  char prevHash[SHA1_HEX_LEN + 1] = {0};
  char expectedHex[SHA1_HEX_LEN + 1] = {0};
  unsigned int difficulty = 0, start = 0, count = 0;

  if (fscanf(f, "%40s", prevHash)    != 1) { fclose(f); return 1; }
  if (fscanf(f, "%40s", expectedHex) != 1) { fclose(f); return 1; }
  if (fscanf(f, "%u",  &difficulty)  != 1) { fclose(f); return 1; }
  if (fscanf(f, "%u",  &start)       != 1) { start = 0; }
  if (fscanf(f, "%u",  &count)       != 1) { count = 0; }
  fclose(f);

  uint8_t expectedDigest[SHA1_LEN];
  if (!hex_to_bytes20_exact(expectedHex, expectedDigest)) { return 1; }

  // limit z difficulty
  unsigned int max_nonce = (difficulty>0U) ? (difficulty-1U) : 0xFFFFFFFFU;

  // brak pracy
  if (start > max_nonce || count == 0U){
    FILE *out = fopen(PAYLOAD_PATH, "w");
    if (out) { fprintf(out, "-1"); fclose(out); }
    return 0;
  }

  // zakres [start..end] (z zabezpieczeniem na overflow)
  unsigned int end = max_nonce;
  unsigned int tmp = start + (count - 1U);
  if (tmp >= start && tmp <= max_nonce) end = tmp;

  // przygotowanie wejścia
  size_t prevLen = my_strlen(prevHash);      // <= 40
  char   inputBuf[40 + 10 + 1];              // prev + nonce + NUL
  uint8_t digest[SHA1_LEN];

  unsigned int found=0;
  int ok=0;

  sha1_ctx c;

  for (unsigned int nonce = start; ; ++nonce) {
    if (nonce > end) break;

    my_memcpy(inputBuf, prevHash, prevLen);
    int ndig = u32_to_dec(inputBuf + prevLen, nonce);
    size_t in_len = prevLen + (size_t)ndig;

    // soft SHA-1
    sha1_init(&c);
    sha1_upd(&c, (const uint8_t*)inputBuf, in_len);
    sha1_fin(&c, digest);

    if (digest_eq_20(digest, expectedDigest)) { found=nonce; ok=1; break; }

    // częstszy „oddech”: co 64 iteracje
    if ((nonce & 0x3Fu) == 0u) { delay(0); }
  }

  FILE *out = fopen(PAYLOAD_PATH, "w");
  if (!out) { return 1; }
  if (ok)  { fprintf(out, "%u", found); }
  else     { fprintf(out, "-1"); }
  fclose(out);

  return 0;
}

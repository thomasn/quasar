// $Id: sha1.h,v 1.3 2005/04/05 07:27:51 bpepers Exp $
//
// This file is the public API for Steve Reid's SHA-1 implementation.
// I've cleaned it up to suite the style of code I want and I've
// changed some of the function arguments around and simplified some
// things to just what Quasar needs.
//
// This code is in the public domain (of course!)

#ifndef SHA1_H
#define SHA1_H

#include "quasar_int.h"

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} SHA1_CTX;

#define SHA1_DIGEST_SIZE 20

void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const uint8_t* dt, uint32_t len);
void SHA1Final(SHA1_CTX* context, uint8_t* digest);

#endif // SHA1_H

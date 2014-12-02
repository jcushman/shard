/*
 * This file is Copyright Daniel Silverstone <dsilvers@digital-scurf.org> 2006
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef LIBGFSHARE_H
#define LIBGFSHARE_H


typedef struct _gfshare_ctx gfshare_ctx;

typedef void (*gfshare_rand_func_t)(unsigned char*, unsigned int);

/* This will, by default, use random(). It's not very good so you should
 * replace it (perhaps with a function which reads from /dev/urandom).
 * If you can't be bothered, be sure to srandom() before you use any
 * of the gfshare_ctx_enc_* functions
 */
extern gfshare_rand_func_t gfshare_fill_rand;

/* ------------------------------------------------------[ Preparation ]---- */

int test_sharecount(gfshare_ctx*);

/* Initialise a gfshare context for producing shares */
gfshare_ctx* gfshare_ctx_init_enc(unsigned char* /* sharenrs */,
                                  unsigned int /* sharecount */,
                                  unsigned char /* threshold */,
                                  unsigned int /* size */);

/* Initialise a gfshare context for recombining shares */
gfshare_ctx* gfshare_ctx_init_dec(unsigned char* /* sharenrs */,
                                  unsigned int /* sharecount */,
                                  unsigned int /* size */);

/* Free a share context's memory. */
void gfshare_ctx_free(gfshare_ctx* /* ctx */);

/* --------------------------------------------------------[ Splitting ]---- */

/* Provide a secret to the encoder. (this re-scrambles the coefficients) */
void gfshare_ctx_enc_setsecret(gfshare_ctx* /* ctx */,
                               unsigned char* /* secret */);

/* Extract a share from the context. 
 * 'share' must be preallocated and at least 'size' bytes long.
 * 'sharenr' is the index into the 'sharenrs' array of the share you want.
 */
void gfshare_ctx_enc_getshare(gfshare_ctx* /* ctx */,
                              unsigned char /* sharenr */,
                              unsigned char* /* share */);

/* ----------------------------------------------------[ Recombination ]---- */

/* Inform a recombination context of a change in share indexes */
void gfshare_ctx_dec_newshares(gfshare_ctx* /* ctx */,
                               unsigned char* /* sharenrs */);

/* Provide a share context with one of the shares.
 * The 'sharenr' is the index into the 'sharenrs' array
 */
void gfshare_ctx_dec_giveshare(gfshare_ctx* /* ctx */,
                               unsigned char /* sharenr */,
                               unsigned char* /* share */);

/* Extract the secret by interpolation of the shares.
 * secretbuf must be allocated and at least 'size' bytes long
 */
void gfshare_ctx_dec_extract(gfshare_ctx* /* ctx */,
                             unsigned char* /* secretbuf */);

#endif /* LIBGFSHARE_H */


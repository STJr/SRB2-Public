/*
 * Copyright (c) 2000-2005 Marc Alexander Lehmann <schmorp@schmorp.de>
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *   3.  The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License version 2 (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of the above. If you wish to
 * allow the use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * BSD license, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL. If
 * you do not delete the provisions above, a recipient may use your version
 * of this file under either the BSD or the GPL.
 */

/* LZF decompression routines copied from lzf_d.c from liblzf 1.7 */

/*
 * lzfP.h included here by Graue.
 */

#ifndef LZFP_h
#define LZFP_h

#include "lzf.h"
#include "m_misc.h"

/*
 * Size of hashtable is (1 << HLOG) * sizeof (char *)
 * decompression is independent of the hash table size
 * the difference between 15 and 14 is very small
 * for small blocks (and 14 is usually a bit faster).
 * For a low-memory/faster configuration, use HLOG == 13;
 * For best compression, use 15 or 16 (or more).
 */
#ifndef HLOG
# define HLOG 15
#endif

/*
 * Sacrifice very little compression quality in favour of compression speed.
 * This gives almost the same compression as the default code, and is
 * (very roughly) 15% faster. This is the preferable mode of operation.
 */

#ifndef VERY_FAST
# define VERY_FAST 1
#endif

/*
 * Sacrifice some more compression quality in favour of compression speed.
 * (roughly 1-2% worse compression for large blocks and
 * 9-10% for small, redundant, blocks and >>20% better speed in both cases)
 * In INT16: when in need for speed, enable this for binary data,
 * possibly disable this for text data.
 */
#ifndef ULTRA_FAST
# define ULTRA_FAST 0
#endif

/*
 * Unconditionally aligning does not cost very much, so do it if unsure
 */
#ifndef STRICT_ALIGN
# define STRICT_ALIGN !(defined(__i386) || defined (__amd64))
#endif

/*
 * Use string functions to copy memory.
 * this is usually a loss, even with glibc's optimized memcpy
 */
#ifndef USE_MEMCPY
# define USE_MEMCPY 0
#endif

/*
 * You may choose to pre-set the hash table (might be faster on some
 * modern cpus and large (>>64k) blocks)
 */
#ifndef INIT_HTAB
# define INIT_HTAB 0
#endif

/*
 * Avoid assigning values to errno variable? for some embedding purposes
 * (linux kernel for example), this is neccessary. NOTE: this breaks
 * the documentation in lzf.h.
 */
#ifndef AVOID_ERRNO
# define AVOID_ERRNO 0
#endif

/*
 * Wether to pass the LZF_STATE variable as argument, or allocate it
 * on the stack. For small-stack environments, define this to 1.
 * NOTE: this breaks the prototype in lzf.h.
 */
#ifndef LZF_STATE_ARG
# define LZF_STATE_ARG 0
#endif

/*
 * Wether to add extra checks for input validity in lzf_decompress
 * and return EINVAL if the input stream has been corrupted. This
 * only shields against overflowing the input buffer and will not
 * detect most corrupted streams.
 * This check is not normally noticable on modern hardware
 * (<1% slowdown), but might slow down older cpus considerably.
 */
#ifndef CHECK_INPUT
# define CHECK_INPUT 1
#endif

/*****************************************************************************/
/* nothing should be changed below */

typedef unsigned char u8;

typedef const u8 *LZF_STATE[1 << (HLOG)];

#if !STRICT_ALIGN
/* for unaligned accesses we need a 16 bit datatype. */
# include <limits.h>
# if USHRT_MAX == 65535
    typedef unsigned short u16;
# elif UINT_MAX == 65535
    typedef unsigned int u16;
# else
#  undef STRICT_ALIGN
#  define STRICT_ALIGN 1
# endif
#endif

#if ULTRA_FAST
# if defined(VERY_FAST)
#  undef VERY_FAST
# endif
#endif

#if USE_MEMCPY || INIT_HTAB
# ifdef __cplusplus
#  include <cstring>
# else
#  include <string.h>
# endif
#endif

#endif


/*
 * lzfP.h ends here.
 */

#if AVOID_ERRNO || defined(_WIN32_WCE)
# define SET_ERRNO(n)
#else
# include <errno.h>
# define SET_ERRNO(n) errno = (n)
#endif

size_t
lzf_decompress (const void *const in_data,  size_t in_len,
                void             *out_data, size_t out_len)
{
  u8 const *ip = (const u8 *)in_data;
  u8       *op = (u8 *)out_data;
  u8 const *const in_end  = ip + in_len;
  u8       *const out_end = op + out_len;

  do
    {
      unsigned int ctrl = *ip++;

      if (ctrl < (1 << 5)) /* literal run */
        {
          ctrl++;

          if (op + ctrl > out_end)
            {
              SET_ERRNO (E2BIG);
              return 0;
            }

#if CHECK_INPUT
          if (ip + ctrl > in_end)
            {
              SET_ERRNO (EINVAL);
              return 0;
            }
#endif

#if USE_MEMCPY
          M_Memcpy (op, ip, ctrl);
          op += ctrl;
          ip += ctrl;
#else
          do
            *op++ = *ip++;
          while (--ctrl);
#endif
        }
      else /* back reference */
        {
          unsigned int len = ctrl >> 5;

          u8 *ref = op - ((ctrl & 0x1f) << 8) - 1;

#if CHECK_INPUT
          if (ip >= in_end)
            {
              SET_ERRNO (EINVAL);
              return 0;
            }
#endif
          if (len == 7)
            {
              len += *ip++;
#if CHECK_INPUT
              if (ip >= in_end)
                {
                  SET_ERRNO (EINVAL);
                  return 0;
                }
#endif
            }

          ref -= *ip++;

          if (op + len + 2 > out_end)
            {
              SET_ERRNO (E2BIG);
              return 0;
            }

          if (ref < (u8 *)out_data)
            {
              SET_ERRNO (EINVAL);
              return 0;
            }

          *op++ = *ref++;
          *op++ = *ref++;

          do
            *op++ = *ref++;
          while (--len);
        }
    }
  while (ip < in_end);

  return op - (u8 *)out_data;
}

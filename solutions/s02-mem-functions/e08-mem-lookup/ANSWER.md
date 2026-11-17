# s02-e08-mem-lookup: the sentences

Quoted from Linux man-pages 6.19.

## memcpy.returns

Page and section: memcpy(3), RETURN VALUE

> The memcpy() function returns a pointer to dest.

`dst`. Every one of `memcpy`, `memmove`, `memset`, `strcpy` and `strcat` returns its
destination, which is the pointer the caller already had — useful only for chaining
(`strlen(memcpy(buf, src, n))`) and never as an error indication. None of them can fail.

## memmove.returns

Page and section: memmove(3), RETURN VALUE

> The memmove() function returns a pointer to dest.

## memchr.not_found

Page and section: memchr(3), RETURN VALUE

> The memchr() ... function returns a pointer to the matching byte or NULL if the character
> does not occur in the given memory area.

`null`.

## memcmp.zero_n

Page and section: memcmp(3), DESCRIPTION

> The memcmp() function compares the first n bytes (each interpreted as unsigned char) of the
> memory areas s1 and s2.

The page states no special case for `n == 0`, and none is needed: with no bytes to compare
there is no first differing byte, so the areas are equal and the return value is `0`. (C17
7.24.4.1 reaches the same answer the same way; 7.24.1p2 adds that a null pointer is still not
allowed, even when `n` is zero.)

## memcpy.restrict

Page and section: memcpy(3), SYNOPSIS

> void *memcpy(size_t n;
>              void dest[restrict n], const void src[restrict n],
>              size_t n);

`yes`. The man-pages write the parameters in array notation with the qualifier inside the
brackets; the C standard's own declaration is
`void *memcpy(void * restrict s1, const void * restrict s2, size_t n);` and macOS writes it
that way too. Same promise, three spellings.

## memmove.restrict

Page and section: memmove(3), SYNOPSIS

> void *memmove(size_t n;
>               void dest[n], const void src[n], size_t n);

`no`: no `restrict` on either parameter. The difference between these two SYNOPSIS lines is
the entire difference between the two functions.

## memcpy.overlap_ok

Page and section: memcpy(3), DESCRIPTION

> The memcpy() function copies n bytes from memory area src to memory area dest. The memory
> areas must not overlap. Use memmove(3) if the memory areas do overlap.

`no`. CAVEATS says it again, with the authority of a standard behind it:

> (POSIX and the C standards are explicit that employing memcpy() with overlapping areas
> produces undefined behavior.)

## memcmp.compares_as

Page and section: memcmp(3), DESCRIPTION

> The memcmp() function compares the first n bytes (each interpreted as unsigned char) of the
> memory areas s1 and s2.

`unsigned char`. This is the sentence that makes `memcmp("\xFF", "\x01", 1)` positive on every
platform.

## memset.converts_to

Page and section: memset(3), DESCRIPTION

> The memset() function fills the first n bytes of the memory area pointed to by s with the
> constant byte c.

The Linux page says "constant byte", and the conversion is explicit in the C standard
(C17 7.24.6.1: "converts the value c to unsigned char"); the macOS page words it as "byte c
(converted to an unsigned char)". `unsigned char` — which is why `memset(p, -1, n)` fills with
0xFF and `memset(p, 0x141, n)` fills with 'A'.

## Two questions with no key

1. **The glibc 2.13 story**, from memcpy(3) CAVEATS. A performance change to `memcpy` on
   x86-64 and some other platforms altered the order in which bytes were copied from source to
   destination. Programs that had been calling `memcpy` with overlapping areas — undefined
   behaviour, but behaviour the old copy order had happened to make harmless — produced wrong
   results, and glibc 2.14 added a versioned symbol so that binaries linked against earlier
   versions kept an implementation aliased to `memmove`.

   What it proves: code that relies on undefined behaviour is not "working", it is *passing*.
   Nothing had changed about the program; the only thing that changed was a library's internal
   choice, which it was always entitled to make. A sanitizer run would have found every one of
   those call sites years earlier — which is what e03 of this unit is.

2. **Why `memcpy` exists.** Because the promise buys speed. `restrict` tells the compiler that
   writes through `dest` cannot change what `src` points at, so it may keep source bytes in
   registers across stores, reorder loads and stores freely, vectorise without an aliasing
   check, and — for small constant sizes — expand the whole call inline into a handful of wide
   loads and stores. `memmove` has to compare the two pointers at run time and choose a
   direction, and a compiler expanding it inline must preserve that. In practice the
   difference is small for large copies and real for the short fixed-size ones that dominate
   struct copying, which is why the compiler emits `memcpy` for a struct assignment and never
   `memmove`.

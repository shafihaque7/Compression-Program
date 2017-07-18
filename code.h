# ifndef _CODE_H
# define _CODE_H

# include <stdint.h>
# include <stdbool.h>

typedef struct code
{
	uint8_t *bits; // The bits are allocated in newCode.
	uint32_t l;
	uint64_t size; // Added the size.
} code;

static inline code *newCode() // returns the pointer.
{
	code *t = malloc(sizeof(code));
	t->bits = calloc( 32, sizeof(uint8_t));
	t->l = 0;
	t->size = 256;
	return t;
}

static inline bool emptyCode( code *c ) { return c->l == 0; }

static inline bool fullCode ( code *c) { return c->l == c->size; }

static inline bool pushCode ( code *c, uint32_t k)
{
	if ( fullCode( c ))
	{
		return false;
	}
	else if( k == 0 )
	{
		c-> bits [c->l / 8] &= ~(0x1 << (c->l % 8));
		c->l += 1;
	}
	else
	{
		c->bits[c->l / 8] |= (0x1 << (c->l % 8 ));
		c->l += 1;
	}
	return true;
}

static inline bool popCode ( code *c, uint32_t *k)
{
	if ( emptyCode( c ) )
	{
		return false;
	}
	else
	{
		c->l -= 1;
		*k = ((0x1 << (c->l % 8)) & c-> bits [c->l / 8]) >> (c->l % 8);
		return true;
	}
}

static inline void appendCode(code *c, code *appendPart) // This one the TA helped us a serious amount.
{
	for( uint32_t i = 0; i < appendPart->l; i++ ) // Goes from zero to the length.
	{
		int valueBit = (appendPart->bits[ i/8 ] & ( 0x1 << ( i % 8 ))) >> ( i % 8 ); // This is like val bit. Gets the value.
		bool checkFull = pushCode( c, valueBit ); // push code returns boolean.
		if ( !checkFull ) // If its not false.
		{
			c->size += 256; // Increments the size.
			c->bits = realloc( c->bits, (c->size / 8) + 1 ); // Reallocates the bits.
			pushCode(c,valueBit); // Pushes the value of the bit.
		}
	}
}


# endif

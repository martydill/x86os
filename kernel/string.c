
/*
* String.c
* Run-time library string handling routines
*/

#include <kernel.h>
#include <console.h>

/* Returns the length of the requested string */
int Strlen(const char* string)
{
    int length = 0;

    if(string == NULL)
        return 0;

    while(*string++ != '\0')
        length++;

    return length;
}


/* Determines whether the two strings are equal */
int Strcmp(const char* string1, const char* string2)
{
    if(Strlen(string1) != Strlen(string2))
        return 1;

    while(*string1 != 0)
    {
        if(*string1++ != *string2++)
            return 1;
    }

    return 0;
}


#define STATE_NORMAL				0
#define STATE_AWAITING_SPECIFIER 	1
#define STATE_ESCAPE_SEQUENCE		2
#define STATE_HAVE_SPECIFIER		3

#define SPECIFIER_INT			1
#define SPECIFIER_STRING		2
#define	SPECIFIER_CHAR			3
#define SPECIFIER_UINT			4

/* Copies a format specifier'd string to another string. Private. */
/* FIXME: Count characters and handle size */
/* FIXME: Can't end on a %s */
STATUS DoSprintf(int size, char* buffer, const char* format, va_list args)
{
    int numSpacesToAdd = 0;
    int index = 0;
    int digit = 0;
    int digitSize = 0;
    int i = 0;
    int state = STATE_NORMAL;
    char currentSpecifier = 0;

    int vaNum = 0, sizeNum = 0;
    char* vaString = 0;
    char vaChar = 0;
    unsigned int vaUInt = 0, usizeNum = 0;

    Assert(buffer != NULL);
    Assert(format != NULL);

    /* Main parsing loop ... */
    while(*format != '\0')
    {
        /* Nothing special is happening */
        if(state == STATE_NORMAL)
        {
            switch(*format)
            {
            case '%':
                /* The start of a format specifier */
                state = STATE_AWAITING_SPECIFIER;
                break;

            case '\\':
                /* Escape sequence */
                state = STATE_ESCAPE_SEQUENCE;
                break;

            case '\t':
                numSpacesToAdd = 4 - index % 4;
                for(int i = 0; i < numSpacesToAdd; ++i) {
                    buffer[index++] = ' ';
                }
                break;

            default:
                /* Just a regular character. Copy it into the buffer. */
                buffer[index]  = *format;
                ++index;
                break;
            }

            ++format;
        }

        /* Waiting for a specifier ( 'd', 's', etc.) */
        else if(state == STATE_AWAITING_SPECIFIER)
        {
            switch(*format)
            {
            case 'd':	/* integer */
                currentSpecifier = SPECIFIER_INT;
                break;

            case 's':	/* string */
                currentSpecifier = SPECIFIER_STRING;
                break;

            case 'c':	/* char */
                currentSpecifier = SPECIFIER_CHAR;
                break;

            case 'u':	/* uint */
                currentSpecifier = SPECIFIER_UINT;
                break;

            default:
                /* Something we don't support ... */
                break;
            }

            state = STATE_HAVE_SPECIFIER;
        }

        /* Currently parsing an escape sequence */
        else if(state == STATE_ESCAPE_SEQUENCE)
        {

        }

        /* We have a format specifier, so we need to read from the VarArg list */
        else if(state == STATE_HAVE_SPECIFIER)
        {
            /* Figure out which specifier it is, and perform the appropriate action */
            switch(currentSpecifier)
            {
            case SPECIFIER_INT:

                vaNum = va_arg(args, int);

                /* If necessary, place a minus sign in front */
                if(vaNum < 0)
                {
                    buffer[index++] = '-';
                    vaNum = -vaNum;
                }

                sizeNum = vaNum;
                digitSize = 0;

                /* Count the number of digits in the number ... */
                do
                {
                    ++digitSize;
                    sizeNum /= 10;
                }
                while(sizeNum > 0);

                /* Copy each digit into the buffer */
                for(i = 0; i < digitSize; ++i)
                {
                    digit = vaNum % 10;
                    vaNum /= 10;
                    buffer[index + digitSize - i - 1] = (char)('0' + digit);
                }

                index += digitSize;
                break;

            case SPECIFIER_STRING:
                vaString = va_arg(args, char*);

                /* FIXME: is this right? */
                /* Just use a space for a NULL pointer */
                if(vaString == NULL)
                {
                    buffer[index++] = ' ';
                    break;
                }

                /* Copy from the VarArg to the buffer */
                while(*vaString != '\0')
                    buffer[index++] = *vaString++;

                break;

            case SPECIFIER_CHAR:
                vaChar = va_arg(args, char);
                buffer[index++] = vaChar;
                break;

            case SPECIFIER_UINT:
                vaUInt = va_arg(args, unsigned int);

                usizeNum = vaUInt;
                digitSize = 0;

                /* Count the number of digits in the number ... */
                do
                {
                    ++digitSize;
                    usizeNum /= 10;
                }
                while(usizeNum > 0);

                /* Copy each digit into the buffer */
                for(i = 0; i < digitSize; ++i)
                {
                    digit = vaUInt % 10;
                    vaUInt /= 10;
                    buffer[index + digitSize - i - 1] = (char)('0' + digit);
                }

                index += digitSize;
                break;

            default:
                break;
            }

            ++format;
            state = STATE_NORMAL;
            currentSpecifier = 0;
        }

    }

    /* Add null terminator */
    buffer[index] = '\0';

    return S_OK;
}


/* Writes a format specifier'd string to another string */
STATUS Sprintf(int size, char* buffer, const char* format, ...)
{
    va_list args;

    if(format == NULL || buffer == NULL)
        return S_FAIL;

    /* Create our VarArgs parser */
    va_start(args, format);
    DoSprintf(size, buffer, format, args);
    va_end(args);

    return S_OK;
}


/* Copies a string to another string */
STATUS Strcpy(char* dest, const char* src, int size)
{
    int i;

    if(dest == NULL || src == NULL)
        return S_FAIL;

    /* FIXME: Optimize this */
    for(i = 0; i < size; ++i)
    {
        *dest++ = *src++;
    }

    return S_OK;
}


void Test_Strlen()
{
    char* p;

    p = "a";
    Assert(Strlen(p) == 1);
    p = "four";
    Assert(Strlen(p) == 4);
    p = "asdf1234asdf12345";
    Assert(Strlen(p) == 17);
}

void Test_Strcmp()
{
    char* p;
    char* q;

    p = "first";
    q = "second";
    Assert(Strcmp(p, q) != 0);
    q = "first123";
    Assert(Strcmp(p, q) != 0);
    q = "first";
    Assert(Strcmp(p, q) == 0);
    p = "dev";
    q = "d9v";
    Assert(Strcmp(p, q) != 0);
    Assert(Strcmp(NULL, NULL) == 0);
}

void Test_Strcpy()
{
    char* p;
    char buf[64];

    p = "test";
    Assert(Strcpy(buf, p, sizeof(buf)) == S_OK);
    Assert(Strcpy(buf, NULL, sizeof(buf)) == S_FAIL);
    Assert(Strcpy(NULL, buf, sizeof(buf)) == S_FAIL);
    Assert(Strlen(buf) == Strlen(p));
    Assert(Strcmp(buf, p) == 0);
    Assert(Strcmp(buf, "test") == 0);
    Assert(Strcmp(buf, "test2") == 1);
}

void Test_Sprintf()
{
    char buf[64];
    unsigned int u = 99;
    Assert(Sprintf(sizeof(buf), buf, "asdf") == S_OK);
    Assert(Strcmp(buf, "asdf") == 0);

    Assert(Sprintf(sizeof(buf), buf, "%s", "hello") == S_OK);
    Assert(Strcmp(buf, "hello") == 0);

    Assert(Sprintf(sizeof(buf), buf, "%d", 12345) == S_OK);
    Assert(Strcmp(buf, "12345") == 0);

    Assert(Sprintf(sizeof(buf), buf, "hello %s goodbye %d world", "asdf", 1234) == S_OK);
    Assert(Strcmp(buf, "hello asdf goodbye 1234 world") == 0);

    Assert(Sprintf(sizeof(buf), buf, "%d", 0) == S_OK);
    Assert(Strcmp(buf, "0") == 0);

    Assert(Sprintf(sizeof(buf), buf, "%c", 'X') == S_OK);
    Assert(Strcmp(buf, "X") == 0);

    Assert(Sprintf(sizeof(buf), buf, "%u", 12345) == S_OK);
    Assert(Strcmp(buf, "12345") == 0);

    Assert(Sprintf(sizeof(buf), buf, "Alloc %u\n", u) == S_OK);
    Assert(Strcmp(buf, "Alloc 99\n") == 0);
}

void Test_String()
{
    Test_Strlen();
    Test_Strcmp();
    Test_Strcpy();
    Test_Sprintf();

    return;
}

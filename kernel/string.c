
/*
 * String.c
 * Run-time library string handling routines
 * 
 * This is shared between kernel and compiled apps
 */

#include <kernel.h>
#include <console.h>

/* Returns the length of the requested string */
int strlen(const char* string) {
  int length = 0;

  if (string == NULL)
    return 0;

  while (*string++ != '\0')
    length++;

  return length;
}

/* Determines whether the two strings are equal */
int strcmp(const char* string1, const char* string2) {
  if (strlen(string1) != strlen(string2))
    return 1;

  while (*string1 != 0) {
    if (*string1++ != *string2++)
      return 1;
  }

  return 0;
}

#define STATE_NORMAL 0
#define STATE_AWAITING_SPECIFIER 1
#define STATE_ESCAPE_SEQUENCE 2
#define STATE_HAVE_SPECIFIER 3

#define SPECIFIER_INT 1
#define SPECIFIER_STRING 2
#define SPECIFIER_CHAR 3
#define SPECIFIER_UINT 4


/* Copies a format specifier'd string to another string. Private. */
/* FIXME: Count characters and handle size */
/* FIXME: Can't end on a %s */
STATUS Dosprintf(int size, char* buffer, const char* format, va_list args) {
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

  /* Main parsing loop ... */
  while (*format != '\0') {
    /* Nothing special is happening */
    if (state == STATE_NORMAL) {
      switch (*format) {
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
        for (int i = 0; i < numSpacesToAdd; ++i) {
          buffer[index++] = ' ';
        }
        break;

      default:
        /* Just a regular character. Copy it into the buffer. */
        buffer[index] = *format;
        ++index;
        break;
      }

      ++format;
    }

    /* Waiting for a specifier ( 'd', 's', etc.) */
    else if (state == STATE_AWAITING_SPECIFIER) {
      switch (*format) {
      case 'd': /* integer */
        currentSpecifier = SPECIFIER_INT;
        break;

      case 's': /* string */
        currentSpecifier = SPECIFIER_STRING;
        break;

      case 'c': /* char */
        currentSpecifier = SPECIFIER_CHAR;
        break;

      case 'u': /* uint */
        currentSpecifier = SPECIFIER_UINT;
        break;

      default:
        /* Something we don't support ... */
        break;
      }

      state = STATE_HAVE_SPECIFIER;
    }

    /* Currently parsing an escape sequence */
    else if (state == STATE_ESCAPE_SEQUENCE) {

    }

    /* We have a format specifier, so we need to read from the VarArg list */
    else if (state == STATE_HAVE_SPECIFIER) {
      /* Figure out which specifier it is, and perform the appropriate action */
      switch (currentSpecifier) {
      case SPECIFIER_INT:

        vaNum = va_arg(args, int);

        /* If necessary, place a minus sign in front */
        if (vaNum < 0) {
          buffer[index++] = '-';
          vaNum = -vaNum;
        }

        sizeNum = vaNum;
        digitSize = 0;

        /* Count the number of digits in the number ... */
        do {
          ++digitSize;
          sizeNum /= 10;
        } while (sizeNum > 0);

        /* Copy each digit into the buffer */
        for (i = 0; i < digitSize; ++i) {
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
        if (vaString == NULL) {
          buffer[index++] = ' ';
          break;
        }

        /* Copy from the VarArg to the buffer */
        while (*vaString != '\0')
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
        do {
          ++digitSize;
          usizeNum /= 10;
        } while (usizeNum > 0);

        /* Copy each digit into the buffer */
        for (i = 0; i < digitSize; ++i) {
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
STATUS sprintf(int size, char* buffer, const char* format, ...) {
  va_list args;

  if (format == NULL || buffer == NULL)
    return S_FAIL;

  /* Create our VarArgs parser */
  va_start(args, format);
  Dosprintf(size, buffer, format, args);
  va_end(args);

  return S_OK;
}

/* Copies a string to another string */
STATUS strcpy(char* dest, const char* src, int size) {
  int i;

  if (dest == NULL || src == NULL)
    return S_FAIL;

  /* FIXME: Optimize this */
  for (i = 0; i < size; ++i) {
    *dest++ = *src++;
  }

  return S_OK;
}

// Search for the given substring in the given string
char* strstr(const char* s1, const char* s2) {
  if (s1 == NULL || s2 == NULL) {
    return NULL;
  }
}

int isalpha(int c) {
  if (c >= 'A' && c <= 'Z') {
    return 1;
  }
  if (c >= 'a' && c <= 'z') {
    return 1;
  }
  return 0;
}

int tolower(int c) {
  if (!isalpha(c)) {
    return c;
  }
  if (c >= 'A' && c <= 'Z') {
    return (c - 'A') + 'a';
  }

  return c;
}

char* strtok(char* str, const char delim) {
  static char* current;
  char* p;
  if (str == NULL) {
    str = current;
    if (current == NULL) {
      return NULL;
    }
  }

  current = str;
  p = str;
  while (*str && *str != delim) {
    str++;
  }
  if (*str) {
    *str = '\0';
    current = str + 1;
  } else {
    current = NULL;
  }

  return p;
}

int atoi (const char * str) {
  int value = 0;
  int sign = 1;

  if(str == NULL) {
    return 0;
  }

  // Move to first non-null non-space character
  while(*str == ' ') str++;

  // For each character...
  while(*str != 0) {
    char currentChar = *str++;

    if(currentChar == '-') {
      sign = -1;
      continue;
    }

    // Invalid character, just return 0
    if(currentChar < '0' || currentChar > '9') {
      return 0;
    }

    // Valid character, convert to digit
    value = value * 10;
    value = value + (int)(currentChar - '0');
  }

  return value * sign;
}


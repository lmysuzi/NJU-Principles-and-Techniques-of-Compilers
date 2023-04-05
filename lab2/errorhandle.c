#include "errorhandle.h"

#define print_error_kind(X, Y) printf("Error type %d at Line %d: ", X, Y)

void throw_error(int error_code, int line, char *object_name)
{
    print_error_kind(error_code, line);
    switch (error_code)
    {
    case 1:
        printf("Undefined variable \"%s\"\n", object_name);
        break;
    case 2:
        printf("Undefined function \"%s\"\n", object_name);
        break;
    case 3:
        printf("Redefined variable \"%s\"\n", object_name);
        break;
    case 4:
        printf("Redefined function \"%s\"\n", object_name);
        break;
    case 5:
        printf("Type mismatched for assignment\n");
        break;
    case 6:
        printf("The left-hand side of an assignment must be a variable\n");
        break;
    case 7:
        printf("Type mismatched for operands\n");
        break;
    case 8:
        printf("Type mismatched for return\n");
        break;
    case 9:
        printf("Function is not applicable for arguments\n");
        break;
    case 10:
        printf("\"%s\" is not an array\n", object_name);
        break;
    case 11:
        printf("\"%s\" is not a function\n", object_name);
        break;
    case 12:
        printf("\"%s\" is not an integer\n", object_name);
        break;
    case 13:
        printf("Illegal use of \".\"\n");
        break;
    case 14:
        printf("Non-existent field \"%s\".\n", object_name);
        break;
    case 15:
        printf("Redefined field \"%s\"\n", object_name);
        break;
    case 16:
        printf("Duplicated name \"%s\"\n", object_name);
        break;
    case 17:
        printf("Undefined structure \"%s\".\n", object_name);
        break;
    case 18:
        printf("Undefined function \"%s\"\n", object_name);
        break;
    case 19:
        printf("Inconsistent definition of function \"%s\"\n", object_name);
        break;
    default:
        printf("wrong error code\n");
        break;
    }
}
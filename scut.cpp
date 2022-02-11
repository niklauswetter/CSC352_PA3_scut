/*
 * Niklaus Wetter
 * CSC 352
 *
 * This class implements a simpler version of the linux cut command, utilizing three different flags
 * and a column selection argument.
 *
 * Delimiter options:
 * -l: letter mode
 * -w: whitespace mode
 * -c: comma mode
 *
 * Selector must begin and end with a digit, contain only commas and dashes
 * digit-digit gives a range of columns
 * digit,digit gives each column discretely
 * digit,digit-digit gives a discrete column followed by a range
 *
 * These different options are all valid and can be combined longer so long as the original restriction is obeyed
 */
//Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Function Declarations
int selectorinterpreter(char* selectorPtr, int* temp);
void lettercutter(char* line, int* selector, int selectorSize);
void spacecutter(char* line, int* selector, int selectorSize);
void commacutter(char* line, int* selector, int selectorSize);

int main(int argc, char *argv[])
{
    //Check that there is exactly 2 command line args
    if(argc!=3)
    {
        fprintf(stderr, "expected 2 command line arguments.\n");
        exit(1);
    }

    //Check that the first argument is a valid delimiter type
    if(strcmp(argv[1], "-l") !=0 && strcmp(argv[1], "-w") != 0 && strcmp(argv[1], "-c") != 0)
    {
        fprintf(stderr, "Invalid delimiter type.\n");
        exit(1);
    }

    //Check that the second argument begins and ends with a digit
    //Check that the second argument only contains digits, commas, dashes
    //Did this in a block to keep the variables in their own scope
    {
        int argLength = strlen(argv[2]);
        if(!isdigit(argv[2][0]) || !isdigit(argv[2][argLength-1]))
        {
            fprintf(stderr, "Invalid selection.\n");
            exit(1);
        }

        for(int i = 0; i < argLength; i++)
        {
            char c = argv[2][i];
            if(c != ',' && c != '-' && !isdigit(c))
            {
                fprintf(stderr, "Invalid selection.\n");
                exit(1);
            }
        }
    }

    //At this point we have verified all arg input is legal
    //Begin taking input
    //Lines will be less than 100 characters
    char linebuf[100];
    //Loop ends at first NULL line
    char selector[strlen(argv[2])];
    strcpy(selector, argv[2]);
    //Store data
    int cols[99];
    int* colsPtr = cols;
    int colsSize;
    colsSize = selectorinterpreter(selector, colsPtr);
    int lines[colsSize];
    int* linesPtr = lines;
    for(int i = 0; i<colsSize;i++)
    {
        lines[i] = cols[i];
    }

    //Decision tree for flag
    if(strcmp(argv[1], "-l") ==0)
    {
        //Letter mode
        while(fgets(linebuf, sizeof(linebuf), stdin) != NULL)
        {
            lettercutter(linebuf, linesPtr, colsSize);
        }
    }
    else if(strcmp(argv[1], "-w") ==0)
    {
        //Whitespace mode
        while(fgets(linebuf, sizeof(linebuf), stdin) != NULL)
        {
            spacecutter(linebuf, linesPtr, colsSize);
        }
    }
    else if(strcmp(argv[1], "-c") ==0)
    {
        //Comma mode
        while(fgets(linebuf, sizeof(linebuf), stdin) != NULL)
        {
            commacutter(linebuf, linesPtr, colsSize);
        }
    }
	return 0;
}

/*
 * Interprets the selector string in the 2nd argument
 * Returns the number of columns that are to be cut
 * Takes a pointer to the selector string to be read, and the array in which to store the values
 * For this project we are allowed to assume no longer than 99 characters, so we assume the array pointer is int[99]
 */
int selectorinterpreter(char* selectorPtr, int* temp)
{
    //Temp will be size int[99]
    int extraLength = 0;
    int tempSize = 0;
    char selector[strlen(selectorPtr)];
    strcpy(selector,selectorPtr);

    int d1,d2,d3,d4;
    int flag = 1;

    int S,D,SS,SD,DD;

    for(int i = 0; flag == 1; i++)
    {
        S = isdigit(selector[i]) && (selector[i+1] == ',' || i == strlen(selector)-1);
        D = isdigit(selector[i]) && isdigit(selector[i+1]) && (selector[i+2] == ',' || (i == strlen(selector)-2));
        SS = isdigit(selector[i]) && selector[i+1] == '-' && isdigit(selector[i+2]) && (selector[i+3] == ',' || i == strlen(selector)-3);
        SD = isdigit(selector[i]) && selector[i+1] == '-' && isdigit(selector[i+2]) && isdigit(selector[i+3]) && (selector[i+4] == ',' || (i == strlen(selector)-4));
        DD = isdigit(selector[i]) && isdigit(selector[i+1]) && selector[i+2] == '-' && isdigit(selector[i+3]) && isdigit(selector[i+4]) && (selector[i+5] == ',' || (i == strlen(selector)-5));

        //Debug line
        //printf("Char Pos: %d;S%d D%d SS%d SD%d DD%d\n",i,S,D,SS,SD,DD);

        if(S)
        {
            //This is a single-digit number with a comma next
            d1 = (int)selector[i]-48;
            *(temp+tempSize) = d1;
            extraLength = 1;
            tempSize+=1;
        }
        else if(D)
        {
            //This is a double-digit number with a comma next
            d1 = ((int)selector[i]-48)*10;
            d2 = (int)selector[i+1]-48;
            d1+=d2;
            *(temp+tempSize) = d1;
            extraLength = 2;
            tempSize+=1;
        }
        else if(SS)
        {
            //This is a single-digit number with a dash then single-digit number then a comma
            d1 = (int)selector[i]-48;
            d2 = (int)selector[i+2]-48;
            for(int j = d1;j<=d2;j++)
            {
                *(temp+tempSize) = j;
                tempSize++;
            }
            extraLength = 3;
        }
        else if(SD)
        {
            //This is a single-digit number with a dash then a double-digit number then a comma
            d1 = (int)selector[i]-48;
            d2 = ((int)selector[i+2]-48)*10;
            d3 = (int)selector[i+3]-48;
            d2 += d3;
            for(int j = d1;j<=d2;j++)
            {
                *(temp+tempSize) = j;
                tempSize++;
            }
            extraLength = 4;
        }
        else if(DD)
        {
            //This is a double-digit number with a dash then a double-digit number then a comma
            d1 = ((int)selector[i]-48)*10;
            d2 = (int)selector[i+1]-48;
            d3 = ((int)selector[i+3]-48)*10;
            d4 = (int)selector[i+4]-48;
            d1 += d2;
            d3 += d4;
            for(int j = d1;j<=d3;j++)
            {
                *(temp+tempSize) = j;
                tempSize++;
            }
            extraLength = 5;
        }
        else
        {
            flag = 0;
        }
        i+=extraLength;
    }
    return tempSize;
}

/*
 * Takes a line of text and a selector array and cuts out the indicated columns by character position in the line
 */
void lettercutter(char* line, int* selector, int selectorSize)
{
    int t;
    for(int i = 0; i < selectorSize; i++)
    {
        t = *(selector+i) - 1;
        if(t < strlen(line))
        {
            printf("%c ", line[t]);
        }
    }
    printf("\n");
}

/*
 * Takes a line of text and a selector array and cuts out the indicated columns by space
 */
void spacecutter(char* line, int* selector, int selectorSize)
{
    char ** result = NULL;
    char * c = strtok(line, " ");
    int numberSpaces = 0;

    while(c)
    {
        result = realloc(result,sizeof(char*) * ++numberSpaces);
        result[numberSpaces-1] = c;
        c = strtok(NULL, " ");
    }

    result = realloc(result,sizeof(char*) * (numberSpaces+1));
    result[numberSpaces] = 0;

    for(int i = 0; i < (numberSpaces+1); i++)
    {
        if(result[i] == NULL)
        {
            continue;
        }
        for(int j = 0; j < strlen(result[i]); j++)
        {
            if(isspace(result[i][j]))
            {
                result[i][j] = 0;
            }
        }
    }

    int t;
    int tCounter = 0;
    for(int i = 0; i < (numberSpaces+1); i++)
    {
        t = *(selector+tCounter) - 1;
        if(result[i] != NULL && t==i)
        {
            printf("%s", result[i]);
            printf(" ");
            tCounter++;
        }
    }
    printf("\n");
}

/*
 * Takes a line of text and a selector array and cuts out the indicated columns by comma
 */
void commacutter(char* line, int* selector, int selectorSize)
{
    char ** result = NULL;
    char * c = strtok(line, ",");
    int numberSpaces = 0;

    while(c)
    {
        result = realloc(result,sizeof(char*) * ++numberSpaces);
        result[numberSpaces-1] = c;
        c = strtok(NULL, ",");
    }

    result = realloc(result,sizeof(char*) * (numberSpaces+1));
    result[numberSpaces] = 0;

    for(int i = 0; i < (numberSpaces+1); i++)
    {
        if(result[i] == NULL)
        {
            continue;
        }
        for(int j = 0; j < strlen(result[i]); j++)
        {
            if(isspace(result[i][j]))
            {
                result[i][j] = 0;
            }
        }
    }

    int t;
    int tCounter = 0;
    for(int i = 0; i < (numberSpaces+1); i++)
    {
        t = *(selector+tCounter) - 1;
        if(result[i] != NULL && t==i)
        {
            printf("%s", result[i]);
            printf(" ");
            tCounter++;
        }
    }
    printf("\n");
}
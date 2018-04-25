#include <stdio.h>
#include <limits.h>

#define MAXMATRIX 10000 /* Maximum matrix elements */
#define MAXNOME 81

#define key1(A) (A.line)
#define key2(A) (A.column)
#define exch(A, B) { Density tmp = A; A = B; B = tmp; }

/* Struct definitions */
typedef struct {
    unsigned long line, column;
    double value;
} Element;

typedef struct {
    unsigned long line;
    int nelements;
} Density;

typedef struct {
    unsigned long column;
    double value;
} ColumnsInfo;

typedef Element Item;

/* Headers */
void addElement();
void listElements();
void infoMatrix();
void infoLine();
void infoColumn();
void redefineZero();
void insertion(Item a[], int l, int r, int (*less)());
void sort();
int compare1(Item A, Item B);
int compare2(Item A, Item B);
void saveFile();
void loadFile();
void updateLimits(unsigned long, unsigned long);
short removeZeros();
void reevaluateLimits();
void compress();
int partition(Density [], int, int);
int  densityCmp(Density, Density);
int fits(int nelements, int line, double value[], int offset, ColumnsInfo columns[]);
short insertValues(Density densityArr[], double value[], unsigned long index[], short offset[]);

/* Global variables */
char filename[MAXNOME];
static int vecpointer = 0 /* Position in array to be written to */, hasName = 0;
static unsigned long mini = INT_MAX, minj = INT_MAX, maxi = 0, maxj = 0; /* Matrix borderlimits */
static double zero = 0; 
Element matrix[MAXMATRIX]; /* Array that contains matrix elements */


int main(int argc, char **argv) {
    char c;
    if (argc == 2) { /* Reads filename from terminal */
        sscanf(argv[1], "%80s", filename);
        hasName = 1;
        loadFile(); /* Load file input */
    }
    while ((c = getchar()) != 'q') {
        switch(c) {
            case 'a':
                addElement();
                break;
            case 'p':
                listElements();
                break;
            case 'i':
                infoMatrix();
                break;
            case 'l':
                infoLine();
                break;
            case 'c':
                infoColumn();
                break;
            case 'o':
                sort();
                break;
            case 'z':
                redefineZero();
                break;
            case 's':
                compress();
                break;
            case 'w':
                saveFile();
                break;
        }
    }
    return 0;
}

/* Auxiliary Functions */


/**
 * Adds element to matrix
 */
void addElement() {
    unsigned long line, column;
    double value;
    short i;
    Element temp;

    scanf(" %lu %lu %lf", &line, &column, &value); /* Scan element values */
    temp.line = line; temp.column = column; temp.value = value;
    for (i = 0; i < vecpointer; i++) {
        /* Look for already used position */
        if (matrix[i].line == line && matrix[i].column == column) { 
            /* Change position's value */
            matrix[i].value = value;
            if (value == zero)
                vecpointer = removeZeros(); /* Remove zero value */
            return;
        }
    }
    if (temp.value != zero) {
        updateLimits(temp.line, temp.column);
        matrix[vecpointer++] = temp;
    }
}

/**
 * Lists all the elements by the order in which they were inserted
 * Prints elements in format: [<line>;<column>]=<value> 
 */
void listElements() {
    short j;
    if (vecpointer == 0)
        printf("empty matrix\n");
    else
        for (j = 0; j < vecpointer; j++)
            printf("[%lu;%lu]=%0.3f\n", matrix[j].line, matrix[j].column, matrix[j].value);
}

/**
 * Prints matrix general info
 */
void infoMatrix() {
    unsigned long nelem, size;
    double dens;
    if (!vecpointer) { /* if vecpointer == 0 there are no elements inserted in the matrix */
        printf("empty matrix\n");
        return;
    }
    nelem = vecpointer;
    size = (maxi - mini + 1) * (maxj - minj + 1);
    dens = (double) nelem / size * 100;
    printf("[%lu %lu] [%lu %lu] %lu / %lu = %0.3f%%\n", mini, minj, maxi, maxj, nelem, size, dens);
}

/**
 * Prints specific line elements in a row
 */
void infoLine() {
    unsigned long size = maxj - minj + 1, line;
    double values[size]; /* Will save each line position */
    short i, changed = 0;

    scanf(" %lu", &line);
    for (i = 0; i < size; i++) /* Initialize aux array values to current zero */
        values[i] = zero;
    
    for (i = 0; i < vecpointer; i++) /* Sets existing line elements to correct value */
        if (matrix[i].line == line) {
            values[matrix[i].column-minj] = matrix[i].value;
            changed = 1;
        }

    if (!changed)
        printf("empty line");
    else
        for (i = 0; i < size; i++) /* Print line elements, sorted by column */
            printf(" %0.3f", values[i]);
    printf("\n");
}

/**
 * Prints specific column elements information. Uses same format as listElements
 */
void infoColumn() {
    unsigned long size = maxi - mini + 1, column;
    Element values[maxi - mini + 1]; /* Will save each line position */
    short i, changed = 0;

    scanf(" %lu", &column);
    for (i = 0; i < size; i++) { /* Initialize aux array with zero value */
        values[i].line = mini + i;
        values[i].column = column;
        values[i].value = zero;
    }

    for (i = 0; i < vecpointer; i++) /* Add existing values to correct position */
        if (matrix[i].column == column) {
            values[matrix[i].line-mini] = matrix[i];
            changed = 1;
        }

    if (!changed)
        printf("empty column\n");
    else
        for (i = 0; i < size; i++)
            printf("[%lu;%lu]=%0.3f\n", values[i].line, values[i].column, values[i].value);
}


/**
 * Checks if item A is lower than B (line precedence is bigger than column's)
 * 
 * @param Item A    First item
 * @param Item B    Second item
 * @return          True if item A is lower than item B
 */
int compare1(Item A, Item B) {
    return key1(A) < key1(B) || (key2(A) < key2(B) && key1(A) == key1(B));
}

/**
 * Checks if item A is lower than B (column precedence is bigger than line's)
 * 
 * @param Item A    First item
 * @param Item B    Second item
 * @return          True if item A is lower than item B
 */
int compare2(Item A, Item B) {
    return key2(A) < key2(B) || (key1(A) < key1(B) && key2(A) == key2(B));
}

/**
 * Sorts the given array using the insertion sort algorithm
 * 
 * @param Item a[]      Array with values
 * @param int l         Left border
 * @param int r         Right border
 * @param int *less     Compare function used by algorithm
 */
void insertion(Item a[], int l, int r, int (*less)()) {
    int i,j;
    for (i = l+1; i <= r; i++) {
        Item v = a[i];
        j = i-1;
        while (j >= l && less(v, a[j])) {
            a[j+1] = a[j];
            j--;
        }
    a[j+1] = v;
    }
}

/**
 * Sorts the matrix using a compare function according to user input
 */
void sort() {
    char c;
    short i;

    for (i = 0; (c = getchar()) != '\n'; i++); /* Clears buffer */
    
    /* Sort accordingly to user input */
    if (i >= 2) 
        insertion(matrix, 0, vecpointer-1, &compare2);
    else
        insertion(matrix, 0, vecpointer-1, &compare1);
}

/**
 * Changes zero representative value
 */
void redefineZero() {
    double value;
    scanf(" %lf", &value);
    zero = value;
    vecpointer = removeZeros(); /* Removes values equal to new zero */
    reevaluateLimits(); /* Updates matrix border limits */
}

 /**
  * Writes matrix into file
  * File write format:
  *     First line:     <mini> <minj> <maxi> <maxj>\n
  *     Other lines:    a <line> <column> <value>\n
  */
void saveFile() {
    short i;
    char c;
    FILE *f;
    c = getchar(); /*remove whitespace or \n depending on case*/

    if (!hasName || c == ' ') { /* Get new filename */
        scanf("%80s", filename);
        hasName = 1;
    }

    /* Write formatted input to file */
    f = fopen(filename, "w");
    fprintf(f, "%lu %lu %lu %lu\n", mini, minj, maxi, maxj); /* Write matrix border limits */
    for (i = 0; i < vecpointer; i++) /* Write all matrix elements */
        fprintf(f, "%lu %lu %f\n", matrix[i].line, matrix[i].column, matrix[i].value);
    fclose(f);
}

/**
 * Reads matrix from file as if written by saveFile()
 */
void loadFile() {
    unsigned long line, column;
    double value;
    Element temp;
    FILE *f;

    f = fopen(filename, "r");
    if (f == NULL) { /* Checks if file exists */
        fclose(f);
        return;
    }

    fscanf(f, "%lu %lu %lu %lu\n", &mini, &minj, &maxi, &maxj); /* Get matrix borderlimits */

    while (!feof(f)) { /* Reads input from file until EOF */
        fscanf(f, "%lu %lu %lf\n", &line, &column, &value);
        temp.line = line; temp.column = column; temp.value = value;
        matrix[vecpointer++] = temp;
    }
    fclose(f);
}   

/**
 * Updates matrix borderlimits
 * @param unsigned long line      Possible new line limit
 * @param unsigned long column    Possible new column limit
 */
void updateLimits(unsigned long line, unsigned long column) {
    if (mini > line) mini = line;
    if (minj > column) minj = column;
    if (maxi < line) maxi = line;
    if (maxj < column) maxj = column;
}

/**
 * Removes all zero values from matrix
 */
short removeZeros() {
    short i, j = 0;
    for (i = 0; i < vecpointer; i++)
        if (matrix[i].value != zero)
            matrix[j++] = matrix[i]; /* j only increments if value is different than zero. Zero values get overwritten or become out of bounds */
    return j;
}

/**
 * Searches the matrix performing a full evaluation of matrix borderlimits
 */
void reevaluateLimits() {
    short i;
    mini = maxi = matrix[0].line;
    minj = maxj = matrix[0].column;

    for (i = 1; i < vecpointer; i++)
        updateLimits(matrix[i].line, matrix[i].column);

}


/**
 * Compares if a is lower than b
 * @param Density a     First element
 * @param Density b     Second element
 */
int densityCmp(Density a, Density b) {
    return a.nelements < b.nelements || (a.nelements == b.nelements && a.line > b.line);
}

int partition(Density a[], int l, int r) {
    int i = l-1;
    int j = r;
    Density v = a[r];
    while (i < j) {
        while (densityCmp(a[++i], v));
        while (densityCmp(v, a[--j]))
            if (j == l)
            break;
        if (i < j)
            exch(a[i], a[j]);
    }
    exch(a[i], a[r]);
    return i;
}


/**
 * Sorts a Density array
 * @param Density a[]       Array to be sorted
 * @param int l             Left border
 * @param int r             Right borders
 */    
void sortDensity(Density a[], int l, int r) {
    int i;
    if (r <= l)
        return;
    i = partition(a, l, r);
    sortDensity(a, l, i-1);
    sortDensity(a, i+1, r);
}

/**
 * Compression algorithm for sparse matrix.
 * Shows in a compact way the information of the matrix by using a values, index and offset array
 */
void compress() {
    short i, offset[maxi-mini+1], maxV = 0;
    double value[20000], dens;
    unsigned long index[20000], size;
    Density density[maxi-mini+1];

    size = (maxi - mini + 1) * (maxj - minj + 1);
    dens = (double) vecpointer / size;
    if (dens > 0.5) {
        printf("dense matrix\n");
        return;
    }

    for (i = 0; i < 2*vecpointer; i++) {
        value[i] = zero;
        index[i] = 0;
    }
    
    
    /* Initialize density array */
    for (i = 0; i < maxi-mini+1; i++) {
        density[i].nelements = 0;
        density[i].line = mini+i;
    }

    /* Set densities */
    for (i = 0; i < vecpointer; i++)
        density[matrix[i].line-mini].nelements++;

    sortDensity(density, 0, maxi-mini);

    maxV = insertValues(density, value, index, offset);
    printf("value =");
    for (i = 0; i <= maxj - minj + maxV; i++)
        printf(" %0.3f", value[i]);
    printf("\nindex =");
    for (i = 0; i <= maxj - minj + maxV; i++)
        printf(" %lu", index[i]);
    printf("\noffset =");
    for (i = 0; i < maxi-mini+1; i++)
        printf(" %d", offset[i]);
    printf("\n");
}


short insertValues(Density densityArr[], double value[], unsigned long index[], short offset[]) {
    ColumnsInfo columnInfo[maxi-mini+1][densityArr[maxi-mini].nelements];
    int i, j, lineIndex[maxi-mini+1], current_offset, maxValue, current_line;
    

    /* Array with line indexes to write columns */
    for (i = 0; i < maxi-mini+1;i++)
        lineIndex[i] = 0;
    
    /* Fill columnInfo matrix with column info */
    for (i = 0; i < vecpointer; i++) {
        columnInfo[matrix[i].line-mini][lineIndex[matrix[i].line-mini]].column = matrix[i].column;
        columnInfo[matrix[i].line-mini][lineIndex[matrix[i].line-mini]++].value = matrix[i].value;
    }

    for (i = maxi-mini; i >= 0; i--) {
        current_offset = 0;
        current_line = densityArr[i].line;

        /* Get offset */
        while (!fits(lineIndex[current_line-mini], current_line, value,  current_offset, columnInfo[current_line-mini]))
            current_offset++;

        /* Write info in arrays*/
        for (j = 0; j < lineIndex[current_line - mini]; j++) {
            value[columnInfo[current_line-mini][j].column - minj + current_offset] = columnInfo[current_line-mini][j].value;
            index[columnInfo[current_line-mini][j].column - minj + current_offset] = current_line;
            if (maxValue < current_offset)
                maxValue = current_offset;
        }
        offset[current_line - mini] = current_offset;
    }

    return maxValue;
}


int fits(int nelements, int line, double value[], int offset, ColumnsInfo columns[]) {
    short i;
    for (i = 0; i < nelements; i++)
        if (value[columns[i].column - minj + offset] != zero) /* Filled already column */
            return 0;
    return 1;
}
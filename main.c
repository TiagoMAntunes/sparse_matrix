#include <stdio.h>
#include <limits.h>

#define MAXMATRIX 10000
#define MAXNOME 81

#define key1(A) (A.line)
#define key2(A) (A.column)
#define exch(A, B) {Density tmp = A; A = B; B = tmp;}

/* Struct definitions */
typedef struct {
    unsigned long line, column;
    double value;
} Element;

typedef struct {
    unsigned long line;
    short offset;
} Offset;


/* Headers */
void addElement();
int removeZeros();
void updateLimits(unsigned long, unsigned long);
void listElements();
void infoMatrix();
void infoLine();
void infoColumn();
int columnCompare(Element A, Element B);
void elementSort(Element a[], int l, int r, int (*less)());
int lineCompare(Element A, Element B);
void sort();
int sortLines(Element, Element);
int sortColumns(Element, Element);
void redefineZero();
void reevaluateLimits();
void saveFile();
void loadFile();
void compress();

/* Global variables */
char filename[MAXNOME];
static int vecpointer = 0 /* Position in array to be written to */, hasName = 0;
static unsigned long mini = ULONG_MAX, minj = ULONG_MAX, maxi = 0, maxj = 0; /* Matrix borderlimits */
static double zero = 0; 
Element matrix[MAXMATRIX]; /* Array that contains matrix elements */


int main(int argc, char **argv) {
    char c;
    if (argc == 2) { /* Reads filename from terminal */
        sscanf(argv[1], "%80s", filename);
        hasName = 1;
        loadFile();
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

void addElement() {
    unsigned long line, column;
    double value;
    Element tmp;
    short i;

    scanf(" %lu %lu %lf\n", &line, &column, &value);
    for (i = 0; i < vecpointer; i++) 
        if (matrix[i].line == line && matrix[i].column == column) {
            matrix[i].value = value;
            if (value == zero)
                vecpointer = removeZeros();
            reevaluateLimits();
            return;
        }

    if (value != zero) {
        tmp.line = line; tmp.column = column; tmp.value = value;
        updateLimits(line, column);
        matrix[vecpointer++] = tmp;
    }
}

int removeZeros() {
    short i, j = 0;
    for (i = 0; i < vecpointer; i++)
        if (matrix[i].value != zero)
            matrix[j++] = matrix[i]; /* j only increments if value is different than zero. Zero values get overwritten or become out of bounds */
    return j;
}

void updateLimits(unsigned long line, unsigned long column) {
    if (mini > line) mini = line;
    if (minj > column) minj = column;
    if (maxi < line) maxi = line;
    if (maxj < column) maxj = column;
}

void listElements() {
    short i;
    if (vecpointer == 0)
        printf("empty matrix\n");
    else 
        for(i = 0; i < vecpointer; i++)
            printf("[%lu;%lu]=%0.3f\n", matrix[i].line, matrix[i].column, matrix[i].value);
}

void infoMatrix() {
    unsigned long size;
    float dens;
    if (!vecpointer) { /* if vecpointer == 0 there are no elements inserted in the matrix */
        printf("empty matrix\n");
        return;
    }
    size = (maxi - mini + 1) * (maxj - minj + 1);
    dens = (float) vecpointer / size * 100;
    printf("[%lu %lu] [%lu %lu] %d / %lu = %0.3f%%\n", mini, minj, maxi, maxj, vecpointer, size, dens);
}

void infoLine() {
    short pointer = 0, i;
    Element columnElements[vecpointer];
    unsigned long line, columnCount;

    scanf(" %lu", &line);
    
    for (i = 0; i < vecpointer; i++) 
        if (matrix[i].line == line)
            columnElements[pointer++] = matrix[i];
    if (pointer == 0) {
        printf("empty line\n");
        return;
    }
    elementSort(columnElements, 0, pointer-1, &columnCompare);
    
    for (i = 0, columnCount = minj; columnCount <= maxj; columnCount++)
        if (columnCount == columnElements[i].column)
            printf(" %0.3f", columnElements[i++].value);
        else
            printf(" %0.3f", zero);

    printf("\n");
}

void infoColumn() {
    short pointer = 0, i;
    Element lineElements[vecpointer];
    unsigned long column, lineCount;

    scanf(" %lu", &column);
    for (i = 0; i < vecpointer; i++) 
        if (matrix[i].column == column)
            lineElements[pointer++] = matrix[i];
    
    if (pointer == 0) {
        printf("empty column\n");
        return;
    }

    elementSort(lineElements, 0, pointer-1, &lineCompare);
    
    for (i = 0, lineCount = mini; lineCount <= maxi ; lineCount++)
        if (lineCount == lineElements[i].line) {
            printf("[%lu;%lu]=%0.3f\n", lineElements[i].line, lineElements[i].column, lineElements[i].value);
            i++;
        }
        else
            printf("[%lu;%lu]=%0.3f\n", lineCount, column, zero);
}

int columnCompare(Element A, Element B) {
    return A.column <= B.column;
}

int lineCompare(Element A, Element B) {
    return A.line <= B.line;
}

void elementSort(Element a[], int l, int r, int (*less)()) {
    short i,j;
    for (i = l+1; i <= r; i++) {
        Element v = a[i];
        j = i-1;
        while (j >= l && less(v, a[j])) {
            a[j+1] = a[j];
            j--;
        }
    a[j+1] = v;
    }
}

int sortColumns(Element A, Element B) {
    return key2(A) < key2(B) || (key1(A) < key1(B) && key2(A) == key2(B));
}

int sortLines(Element A, Element B) {
    return key1(A) < key1(B) || (key2(A) < key2(B) && key1(A) == key1(B));
}

void sort() {
    char c;
    short i;
    for (i = 0; (c = getchar()) != '\n'; i++);
    if (i > 1)
        elementSort(matrix, 0, vecpointer - 1, &sortColumns);
    else
        elementSort(matrix, 0, vecpointer - 1, &sortLines);
}

void redefineZero() {
    double value;
    scanf(" %lf", &value);
    zero = value;
    vecpointer = removeZeros(); /* Removes values equal to new zero */
    reevaluateLimits(); /* Updates matrix border limits */
}

void reevaluateLimits() {
    short i;
    if (vecpointer == 0) {
        mini = ULONG_MAX, minj = ULONG_MAX, maxi = 0, maxj = 0;
    }
    else {
        mini = maxi = matrix[0].line;
        minj = maxj = matrix[0].column;

        for (i = 1; i < vecpointer; i++)
            updateLimits(matrix[i].line, matrix[i].column);
    }
}

void saveFile() {
    short i;
    char c;
    FILE *f;

    c = getchar();
    if (!hasName || c == ' ') {
        scanf("%80s", filename);
        hasName = 1;
    }

    /* Write formatted input to file */
    f = fopen(filename, "w");
    for (i = 0; i < vecpointer; i++) /* Write all matrix elements */
        fprintf(f, "%lu %lu %f\n", matrix[i].line, matrix[i].column, matrix[i].value);
    fclose(f);
}

void loadFile() {
    Element temp;
    FILE *f;

    f = fopen(filename, "r");
    if (f == NULL) /* Checks if file exists */
        return;

    while (!feof(f)) { /* Reads input from file until EOF */
        fscanf(f, "%lu %lu %lf\n", &temp.line, &temp.column, &temp.value);
        matrix[vecpointer++] = temp;
    }
    fclose(f);
    vecpointer = removeZeros();
    reevaluateLimits();
}

void offsetSort(Offset a[], int l, int r) {
    short i,j;
    for (i = l+1; i <= r; i++) {
        Offset v = a[i];
        j = i-1;
        while (j >= l && (v.line <= a[j].line)) {
            a[j+1] = a[j];
            j--;
        }
    a[j+1] = v;
    }
}

short removeDuplicates(Offset a[], int l, int r) {
    short i, j;
    for (i = l, j = l; i <= r; i++)
        if (a[i].line != a[i+1].line)
            a[j++] = a[i];
    return j;
}

void compress() {
    Element compressed[vecpointer];
    Offset offset[vecpointer];
    short i, pointer;

    if (vecpointer == 0 || ((double) vecpointer / ((maxi-mini+1)*(maxj-minj+1))) > 0.5) {
        printf("dense matrix\n");
        return;
    }

    /* Get all lines */
    for (i = 0; i < vecpointer; i++) {
        offset[i].line = matrix[i].line;
        offset[i].offset = 0;
    }

    /* Sort by line and remove duplicate lines */
    offsetSort(offset, 0, i-1);
    pointer = removeDuplicates(offset, 0, i-1);

    

}
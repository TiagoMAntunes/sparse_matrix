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
    short offset, nelements;
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


/* Add an element to the function, updating its limits */
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

/* Removes all zeros from matrix 
 * @return int - new vecpointer
 */
int removeZeros() {
    short i, j = 0;
    for (i = 0; i < vecpointer; i++)
        if (matrix[i].value != zero)
            matrix[j++] = matrix[i];
    return j;
}

/*  Updates global limits of matrix given an element
 *  @param unsigned long line
 *  @param unsgined long column
 */
void updateLimits(unsigned long line, unsigned long column) {
    if (mini > line) mini = line;
    if (minj > column) minj = column;
    if (maxi < line) maxi = line;
    if (maxj < column) maxj = column;
}

/* Prints matrix elements to stdout */
void listElements() {
    short i;
    if (vecpointer == 0)
        printf("empty matrix\n");
    else 
        for(i = 0; i < vecpointer; i++)
            printf("[%lu;%lu]=%0.3f\n", matrix[i].line, matrix[i].column, matrix[i].value);
}

/* Prints matrix borders and density */
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

/* Prints line elements to stdout */
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
    /* Sorts elements to print them in a sparse way */
    elementSort(columnElements, 0, pointer-1, &columnCompare);

    for (i = 0, columnCount = minj; columnCount <= maxj; columnCount++)
        if (i < pointer && columnCount == columnElements[i].column)
            printf(" %0.3f", columnElements[i++].value);
        else
            printf(" %0.3f", zero);

    printf("\n");
}

/* Prints column elements to stdout */
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

    /* Sorts elements to print them in a sparse way */
    elementSort(lineElements, 0, pointer-1, &lineCompare);
    
    for (i = 0, lineCount = mini; lineCount <= maxi ; lineCount++)
        if (i < pointer && lineCount == lineElements[i].line) {
            printf("[%lu;%lu]=%0.3f\n", lineElements[i].line, lineElements[i].column, lineElements[i].value);
            i++;
        }
        else
            printf("[%lu;%lu]=%0.3f\n", lineCount, column, zero);
}

/*  Returns column of A <= column of B
 *  @param Element A
 *  @param Element B
 *  @return int
 */
int columnCompare(Element A, Element B) {
    return A.column <= B.column;
}

/*  Returns line of A <= line of B
 *  @param Element A
 *  @param Element B
 *  @return int
 */
int lineCompare(Element A, Element B) {
    return A.line <= B.line;
}

/*  Sorts element array using given function
 *  @param Element a[]
 *  @param int l
 *  @param int r
 *  @param int (*less)()
 */
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

/*  Returns A < B sorted by column first
 *  @param Element A
 *  @param Element B
 *  @return int
 */
int sortColumns(Element A, Element B) {
    return key2(A) < key2(B) || (key1(A) < key1(B) && key2(A) == key2(B));
}

/*  Returns A < B sorted by line first
 *  @param Element A
 *  @param Element B
 *  @return int
 */
int sortLines(Element A, Element B) {
    return key1(A) < key1(B) || (key2(A) < key2(B) && key1(A) == key1(B));
}

/* Sorts matrix changing compare function according to user input */
void sort() {
    char c;
    short i;
    for (i = 0; (c = getchar()) != '\n'; i++);
    if (i > 1)
        elementSort(matrix, 0, vecpointer - 1, &sortColumns);
    else
        elementSort(matrix, 0, vecpointer - 1, &sortLines);
}

/* Changes current zero to new zero given by user input, eliminating respective elements */
void redefineZero() {
    double value;
    scanf(" %lf", &value);
    zero = value;
    vecpointer = removeZeros(); /* Removes values equal to new zero */
    reevaluateLimits(); /* Updates matrix border limits */
}

/* Rechecks matrix limits from start */
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
 
/* Saves matrix to 'filelame' file */
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

/* Loads input from 'filename' file if it exists */
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

/*  Returns A < B according to lines 
 *  @param Offset A
 *  @param Offset B
 *  @return int
 */
int offsetLineCmp(Offset A, Offset B) {
    return A.line < B.line;
}

/*  Returns A < B according to line number of elements first and then line index
 *  @param Offset A
 *  @param Offset B
 *  @return int
 */
int offsetAmountCmp(Offset A, Offset B) {
    return A.nelements < B.nelements || (A.nelements == B.nelements && A.line > B.line);
}

/*  Sorts a[] according to given function 
 *  @param Offset a[]
 *  @param int l
 *  @param int r
 *  @param int (*less)()
 */
void offsetSort(Offset a[], int l, int r, int (*less)()) {
    short i,j;
    for (i = l+1; i <= r; i++) {
        Offset v = a[i];
        j = i-1;
        while (j >= l && less(v, a[j])) {
            a[j+1] = a[j];
            j--;
        }
    a[j+1] = v;
    }
}

/*  Removes duplicate lines from a[], making single elements sum nelements
 *  @param Offset a[]
 *  @param int l
 *  @param int r
 */
short removeDuplicates(Offset a[], int l, int r) {
    short i, j;
    for (i = l, j = l; i <= r; i++)
        if (a[i].line != a[i+1].line)
            a[j++].line = a[i].line;
        else
            a[j].nelements++;
    return j;
}

/*  Returns index of element with specified column in array a[] by using binary search algorithm
 *  @param Element a[]
 *  @param int l
 *  @param int r
 *  @param unsigned long column
 *  @return short
 */
short columnSearch(Element a[], int l, int r, unsigned long column) {
    short m = l + (r-l)/2;
    if (l > r) 
        return -1;
    else if (a[m].column == column && a[m].value != zero)
        return m;
    else if (a[m].column < column)
        return columnSearch(a, m+1, r, column);
    else
        return columnSearch(a, 0, m-1, column);
}

/*  Inserts elements[] in compressed[] in an ordered way, using a specified offset
 *  @param Element compressed[]
 *  @param Element elemenets[]
 *  @param short compressedPointer
 *  @param short elementsPointer
 *  @param short offset
 */
void insertOrdered(Element compressed[], Element elements[], short compressedPointer, short elementsPointer, short offset) {
    short i, j, k;
    Element aux[compressedPointer + elementsPointer];
    k = 0; i = 0; j = 0; 
    while (i < compressedPointer && j < elementsPointer) 
        if (compressed[i].column > elements[j].column + offset) {
            aux[k] = elements[j++];
            aux[k++].column += offset;
        } else 
            aux[k++] = compressed[i++];

    while(i < compressedPointer)
        aux[k++] = compressed[i++];
    while(j < elementsPointer) {
        aux[k] = elements[j++];
        aux[k++].column += offset;
    }   

    for (i = 0; i < k; i++)
        compressed[i] = aux[i];
    
}
/*  Returns offset so that elements with specified line can fit in compressed
 *  @param Element compressed[]
 *  @param short compressedPointer
 *  @param short nelements
 *  @param unsigned long line
 *  @return short
 */
short fits(Element compressed[], short compressedPointer, short nelements, unsigned long line) {
    short i, elementsPointer = 0, itFits, current_offset, pos;
    Element elements[nelements];
    
    /* Get elements to use*/
    for (i = 0; i < vecpointer && elementsPointer < nelements; i++) 
        if (matrix[i].line == line)
            elements[elementsPointer++] = matrix[i];
    
    elementSort(elements,0, elementsPointer-1, columnCompare);

    /* Get offset */
    itFits = 0;
    current_offset = 0;
    while (!itFits) {
        itFits = 1;
        for (i = 0; i < elementsPointer; i++) { /* Verify offset insertion */
            pos = columnSearch(compressed, 0, compressedPointer, elements[i].column + current_offset);
            if (pos != -1) { /* Position is filled, get new offset */
                current_offset++;
                itFits = 0;
                break;
            }
        }
    }

    insertOrdered(compressed, elements, compressedPointer, elementsPointer, current_offset);
    return current_offset;
}


/* Displays matrix to stdout using compression algorithm */
void compress() {
    Element compressed[vecpointer];
    Offset offset[vecpointer];
    short i, j, offsetPointer = 0, compressedPointer = 0, current_offset, maxOffset = 0;

    if (vecpointer == 0 || ((double) vecpointer / ((maxi-mini+1)*(maxj-minj+1))) > 0.5) {
        printf("dense matrix\n");
        return;
    }

    /* Get all lines and initialize arrays */
    for (i = 0; i < vecpointer; i++) {
        offset[i].line = matrix[i].line;
        offset[i].offset = 0;
        offset[i].nelements = 1;
        compressed[i].line = 0;
        compressed[i].column = 0;
        compressed[i].value = zero;
    }

    /* Remove duplicate lines and sort by density */
    offsetSort(offset, 0, i-1, &offsetLineCmp);
    offsetPointer = removeDuplicates(offset, 0, i-1);
    offsetSort(offset, 0, offsetPointer-1, &offsetAmountCmp);
        
    /* Algorithm logic. Fills arrays with info */
    for (i = offsetPointer-1; i >= 0; i--) {
        current_offset = fits(compressed, compressedPointer, offset[i].nelements, offset[i].line);
        compressedPointer += offset[i].nelements;
        offset[i].offset = current_offset;
        if (maxOffset < current_offset)
            maxOffset = current_offset;
    }

    offsetSort(offset, 0, offsetPointer-1, &offsetLineCmp);

    /* Print info to stdout */
    printf("value =");
    for (i = minj, j = 0; i < maxj+maxOffset+1; i++) {
        if (j < compressedPointer && i == compressed[j].column)
            printf(" %0.3f", compressed[j++].value);
        else
            printf(" %0.3f", zero);
    }

    printf("\nindex =");
    for (i = minj, j = 0; i < maxj+maxOffset+1; i++){
        if (j < compressedPointer && i == compressed[j].column)
            printf(" %lu", compressed[j++].line);
        else
            printf(" 0");
    }

    printf("\noffset =");
    for (i = mini, j = 0; i < maxi + 1; i++)
        if (j < offsetPointer && i == offset[j].line)
            printf(" %d", offset[j++].offset);
        else
            printf(" 0");
    printf("\n");
}
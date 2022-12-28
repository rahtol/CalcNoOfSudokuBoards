#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sudoku-count.h"
#include <cstring>

typedef candidate_t *(pabc_t [3]);

catalog_entry_t *catalog[36288];
int catalog_size = 0;

int perm_abc[3][3] = {
    { 1, 2, 3 },
    { 2, 1, 3 },
    { 3, 1, 2 }
};

int perm_triple [6][3] = {
    { 0, 1, 2 },
    { 0, 2, 1 },
    { 1, 0, 2 },
    { 1, 2, 0 },
    { 2, 0, 1 },
    { 2, 1, 0 }
};

void generate_entries_from_b2b3sets (block_t b2, pabc_t b2pabc, block_t b3, pabc_t b3pabc, int num_abc)
{
    for (int iabc = 0; iabc < num_abc; iabc++)
    {
        for (int i=0; i<3; i++) *(b2pabc[i]) = perm_abc[iabc][i];
        for (int i=0; i<3; i++) *(b3pabc[i]) = perm_abc[iabc][i];

        for (int i01 = 0; i01 < 6; i01++)
            for (int i02 = 0; i02 < 6; i02++)
                for (int i11 = 0; i11 < 6; i11++)
                    for (int i12 = 0; i12 < 6; i12++)
                    {
                        catalog_entry_t *tmp = new catalog_entry_t();
                        tmp->id = catalog_size;
                        tmp->p_class = tmp; // initially, each catalog_entry is its own class
                        for (int i=0; i<3; i++) tmp->b2[0][i] = b2[0][i];
                        for (int i=0; i<3; i++) tmp->b2[1][i] = b2[1][perm_triple[i01][i]];
                        for (int i=0; i<3; i++) tmp->b2[2][i] = b2[2][perm_triple[i02][i]];
                        for (int i=0; i<3; i++) tmp->b3[0][i] = b3[0][i];
                        for (int i=0; i<3; i++) tmp->b3[1][i] = b3[1][perm_triple[i11][i]];
                        for (int i=0; i<3; i++) tmp->b3[2][i] = b3[2][perm_triple[i12][i]];
                        catalog[catalog_size++] = tmp;
                    }
    }
}

int cmp_block (const block_t &a, const block_t &b)
{
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
        {
            if (a[i][j] == b [i][j]) 
                continue;
            else if (a[i][j] < b [i][j])
                return -1;
            else
                return +1;
        }
    return 0;
}

int cmp_catalog_entry(const catalog_entry_t *a, const catalog_entry_t *b)
{
    int cmpb2 = cmp_block(a->b2, b->b2);
    if (cmpb2 != 0)
        return cmpb2;
    else
        return cmp_block(a->b3, b->b3);
}

int comp (const void *a, const void *b)
{
    return cmp_catalog_entry(*((catalog_entry_t **)a), *((catalog_entry_t **)b));
}

bool check_catalog_order()
{
    for (int i=1; i < catalog_size; i++)
        if (cmp_catalog_entry (catalog[i], catalog[i-1]) <= 0)
            return false;
    return true;
}

bool is_in_row(int n, candidate_t row[3])
{
    return n==row[0] || n==row[1] || n==row[2];
}

void add_to_row(int n, candidate_t row[3])
{
    int i = 2;
    assert(n>0 && n<10);    // assure n is a valid sudoku candidate value
    assert(row[0]==0);      // assure there is space in the row
    while (n < row[i]) 
        i--;
    assert(n!=row[i]);      // assure no duplicates in row
    for(int ii=0; ii<i; ii++)
        row[ii] = row[ii+1];
    row[i] = n;
}

bool check_row (candidate_t row[3], int &rowlen)
{
    bool ok = true;
    int i = 0;
    for (i=0; i<3; i++)
        if (row[i] != 0) break;
    rowlen = 3 - i;
    for (int ii=i; ii<2; ii++)
        ok &= row[ii] < row[ii+1];
//    printf("check_row: %d %d %d %d\n", row[0],row[1],row[2],rowlen);
    return ok;
}

bool check_zeros_nonzeros (block_t b, pabc_t pabc)
{
    bool ok = true;
    ok &= (*(pabc[0]) == 0);
    ok &= (*(pabc[1]) == 0);
    ok &= (*(pabc[2]) == 0);

    int len_row1 = 0;
    int len_row2 = 0;    
    ok &= check_row (b[1], len_row1);
    ok &= check_row (b[2], len_row2);
//    printf("%d %d\n", len_row1, len_row2);
//    ok &= ((len_row1 + len_row2) == 3);

    return ok;
}

void generate_entries_from_b2row0(int b2row0col0, int b2row0col1, int b2row0col2)
{
    block_t b1b2b3[3];

    // initialize with zeros
    for (int b=0; b<3; b++) 
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                b1b2b3[b][i][j] = 0;

    // initialize block #1 in normal form
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            b1b2b3[0][i][j] = 3*i+j+1;

    // block #2, row #0 is given by params
    assert(b2row0col0>0);
    assert(b2row0col1>b2row0col0);
    assert(b2row0col2>b2row0col1);
    assert(10>b2row0col2);
    b1b2b3 [1][0][0] = b2row0col0;
    b1b2b3 [1][0][1] = b2row0col1;
    b1b2b3 [1][0][2] = b2row0col2;

    // block #3, row0 to be filled with missing values
    for (int n=4; n<=9; n++)
        if (!is_in_row(n, b1b2b3[1][0]))
            add_to_row(n, b1b2b3[2][0]);

    // complete the "hidden triple" for n=4 to 9,
    // find the block (#2 or #3) and the row (row1 or row2) where the third n can be placed
    for (int n=4; n<=9; n++)
    {
        // place n to the other block and other row considering rows 1 and 2 of blocks #2 and #3
        add_to_row(n, b1b2b3 [is_in_row(n, b1b2b3[1][0]) ? 2 : 1][(n-1) / 3 == 1 ? 2 : 1]);
    }

    if (b1b2b3[1][2][2] == 0)
    {
        // row2 of block #2 is completely empty
        pabc_t b2pabc = { &b1b2b3[1][2][0], &b1b2b3[1][2][1], &b1b2b3[1][2][2] };
        pabc_t b3pabc = { &b1b2b3[2][1][0], &b1b2b3[2][1][1], &b1b2b3[2][1][2] };
        
        assert(check_zeros_nonzeros (b1b2b3[1], b2pabc));
        assert(check_zeros_nonzeros (b1b2b3[2], b3pabc));

        generate_entries_from_b2b3sets(b1b2b3[1], b2pabc, b1b2b3[2], b3pabc, 1);
    }
    else if (b1b2b3[1][2][1] == 0)
    {
        // row2 of block #2 has two free places
        pabc_t b2pabc = { &b1b2b3[1][1][0], &b1b2b3[1][2][0], &b1b2b3[1][2][1] };
        pabc_t b3pabc = { &b1b2b3[2][2][0], &b1b2b3[2][1][0], &b1b2b3[2][1][1] };
        
        assert(check_zeros_nonzeros (b1b2b3[1], b2pabc));
        assert(check_zeros_nonzeros (b1b2b3[2], b3pabc));

        generate_entries_from_b2b3sets(b1b2b3[1], b2pabc, b1b2b3[2], b3pabc, 3);
    }
    else
    {
        assert(b1b2b3[1][2][0] == 0);
        // row2 of block #2 has one free place
        pabc_t b2pabc = { &b1b2b3[1][2][0], &b1b2b3[1][1][0], &b1b2b3[1][1][1] };
        pabc_t b3pabc = { &b1b2b3[2][1][0], &b1b2b3[2][2][0], &b1b2b3[2][2][1] };
        
        assert(check_zeros_nonzeros (b1b2b3[1], b2pabc));
        assert(check_zeros_nonzeros (b1b2b3[2], b3pabc));

        generate_entries_from_b2b3sets(b1b2b3[1], b2pabc, b1b2b3[2], b3pabc, 3);
    }
}

void generate_catalog()
{
    catalog_size = 0;
    for (int col1=5; col1<9; col1++)
        for(int col2=col1+1; col2<10; col2++)
            generate_entries_from_b2row0(4, col1, col2);
    assert(catalog_size == 36288);
    qsort(catalog, catalog_size, sizeof(catalog_entry_t *), comp);
    assert ( check_catalog_order() );
}

static void copy_block(block_t dest, block_t src)
{
    memcpy(dest, src, sizeof(block_t));
}

int search_catalog_entry(block_t b2, block_t b3)
{
    catalog_entry_t target;
    copy_block(target.b2, b2);
    copy_block(target.b3, b3);

    int low = 0;
    int high = catalog_size;
    // invariant condition: target is in range [low] .. [high-1]
    while (low < high)
    {
        int pivot = (low + high) / 2;  // pivot is always strictly less than high
        int cmp = cmp_catalog_entry (&target, catalog[pivot]);
        if (cmp == 0)
        {
            return pivot;
        }
        else if (cmp < 0)
        {
            // target < [pivot]
            high = pivot;
        }
        else{
            // target > [pivot]
            low = pivot + 1;
        }
    }
    return -1;
}

/************************** TEST CODE *********************************/

#ifdef TEST_SUDOKU_3

block_t b2_457 = {
    {4, 5, 7},
    {0, 8, 9},
    {0, 0, 6}
};
block_t b3_457 = {
    {6, 8, 9},
    {0, 0, 7},
    {0, 4, 5}
};

pabc_t b2pabc = { &b2_457[1][0], &b2_457[2][0], &b2_457[2][1] };
pabc_t b3pabc = { &b3_457[2][0], &b3_457[1][0], &b3_457[1][1] };


void main ()
{
    printf("sudoku-3 #104\n");

    generate_catalog();
    if (!check_catalog_order())
        printf("Illegal catalog order!\n");

    printf("catalog_size=%d\n", catalog_size);

    *b2pabc[0] = 2;
    *b2pabc[1] = 1;
    *b2pabc[2] = 3;
    *b3pabc[0] = 2;
    *b3pabc[1] = 3;
    *b3pabc[2] = 1;
    int idx = search_catalog_entry(b2_457, b3_457);
    printf("serch_catalog_entry = %d\n", idx);
}

#endif

#include "sudoku-count.h"
#include <assert.h>
#include <cstring>
#include <stdio.h>

void swap (candidate_t *a, candidate_t *b)
{
    candidate_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void copy_block(block_t dest, block_t src)
{
    memcpy(dest, src, sizeof(block_t));
}

void init_b1_standard(block_t b1)
{
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            b1[i][j] = 3*i+j+1;
}

catalog_entry_t *get_class_representative(catalog_entry_t *catalog_entry)
{
    assert(catalog_entry != NULL );
    catalog_entry_t *p_class = catalog_entry->p_class;
    while (p_class->p_class != p_class)
        p_class = p_class->p_class;
    return p_class;
}

bool is_class_representative(catalog_entry_t *catalog_entry)
{
    return get_class_representative(catalog_entry) == catalog_entry;
}

int get_no_of_classes()
{
    int no_classes = 0;
    for (int idx=0; idx < catalog_size; idx++)
        if (is_class_representative(catalog[idx]))
        {
            no_classes++;
        }
    return no_classes;
}

int get_class_size(catalog_entry_t *catalog_entry)
{
    assert(catalog_entry != NULL );
    catalog_entry_t *p_class_representative = get_class_representative(catalog_entry);
    int class_size = 0;
    for (int idx=0; idx < catalog_size; idx++)
        if (get_class_representative(catalog[idx]) == p_class_representative)
            class_size++;
    return class_size;
}


void apply_block_permutation(block_t dest[3], block_t src[3], int perm[3])
{
    copy_block(dest[0], src[perm[0]]);
    copy_block(dest[1], src[perm[1]]);
    copy_block(dest[2], src[perm[2]]);
}

void apply_col_permutation(block_t dest, block_t src, int perm[3])
{
    for (int j=0; j<3; j++)
    {
       dest[j][0] = src[j][perm[0]]; 
       dest[j][1] = src[j][perm[1]]; 
       dest[j][2] = src[j][perm[2]]; 
    }
}

void apply_row_permutation(block_t dest[3], block_t src[3], int perm[3])
{
    for (int j=0; j<3; j++)
    {
       dest[0][0][j] = src[0][perm[0]][j]; 
       dest[0][1][j] = src[0][perm[1]][j]; 
       dest[0][2][j] = src[0][perm[2]][j]; 
       dest[1][0][j] = src[1][perm[0]][j]; 
       dest[1][1][j] = src[1][perm[1]][j]; 
       dest[1][2][j] = src[1][perm[2]][j]; 
       dest[2][0][j] = src[2][perm[0]][j]; 
       dest[2][1][j] = src[2][perm[1]][j]; 
       dest[2][2][j] = src[2][perm[2]][j]; 
    }
}

void column_sort (block_t dest, block_t src)
{
    int p[3] = { 0, 0, 0 };
    src[0][0] < src[0][1] ? p[1]++ : p[0]++;
    src[0][2] < src[0][0] ? p[0]++ : p[2]++;
    src[0][1] < src[0][2] ? p[2]++ : p[1]++;
    int p_invers [3];
    p_invers[p[0]] = 0;
    p_invers[p[1]] = 1;
    p_invers[p[2]] = 2;
    apply_col_permutation(dest, src, p_invers);
}

void normalize_b1b2b3 (block_t dest[3], block_t src[3])
{
    block_t tmp[3]; // intermediate storage after permuting candidates but before permuting columns

    // permute candidate values such that b1 in in standard form
    int permute_candidate_values[10]; // indexing is by candidate values 1 .. 9, [0] stays unused 
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            permute_candidate_values [src[0][i][j]] = 3*i+j+1;
    for (int b=0; b<3; b++)
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                tmp[b][i][j] = permute_candidate_values[src[b][i][j]];

    // determine ordering of b2 and b3 - the one containing the 4 in row0 must become b2
    int pblock[3] = {0, 0, 0};
    if (tmp[1][0][0] == 4 || tmp[1][0][1] == 4 || tmp[1][0][2] == 4)
    {
        // first row of b2 contains the minimal value 4 in first row of b2b3
        // -> keep order of b2 and b3 blocks
        pblock[1] = 1;
        pblock[2] = 2;
    }
    else
    {
        // first row of b3 contains the 4
        // -> reverse order of b2 and b3 blocks
        pblock[1] = 2;
        pblock[2] = 1;
    }
    copy_block(dest[0], tmp[0]); // b1 already in standard form
    column_sort (dest[pblock[1]], tmp[1]);     
    column_sort (dest[pblock[2]], tmp[2]);    
}

void span_class_from_one_entry_first_level (catalog_entry_t *p_class)
{
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    // optimization: don't check entries again that already have been assigned to a class in this round
    if (p_class != p_class_representative)
        return;

    block_t lvl0[3], lvl1[3], lvl2[3], lvl3[3], lvl4[3], lvl5[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    for (int i_block_perm=0; i_block_perm < 6; i_block_perm++)
    {
        apply_block_permutation(lvl1, lvl0, perm_triple[i_block_perm]);
        for (int i_col_perm_b1=0; i_col_perm_b1 < 6; i_col_perm_b1++)
        {
            apply_col_permutation(lvl2[0], lvl1[0], perm_triple[i_col_perm_b1]);
            copy_block (lvl2[1], lvl1[1]);
            copy_block (lvl2[2], lvl1[2]);
            for (int i_col_perm_b2=0; i_col_perm_b2 < 6; i_col_perm_b2++)
            {
                copy_block (lvl3[0], lvl2[0]);
                apply_col_permutation(lvl3[1], lvl2[1], perm_triple[i_col_perm_b2]);
                copy_block (lvl3[2], lvl2[2]);
                for (int i_col_perm_b3=0; i_col_perm_b3 < 6; i_col_perm_b3++)
                {
                    copy_block (lvl4[0], lvl3[0]);
                    copy_block (lvl4[1], lvl3[1]);
                    apply_col_permutation(lvl4[2], lvl3[2], perm_triple[i_col_perm_b3]);
                    normalize_b1b2b3 (lvl5, lvl4);
                    int catalog_idx = search_catalog_entry(lvl5[1], lvl5[2]);
                    assert( catalog_idx >=0 ); // assure entry was found
                    catalog_entry_t *p_classmate = catalog[catalog_idx];
                    assert( p_classmate->p_class == p_classmate->p_class || p_classmate->p_class == p_class_representative  );
                    p_classmate->p_class = p_class_representative;
                }
            }
        }
    }
}

void span_class_from_one_entry_second_level(catalog_entry_t *p_class)
{
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    block_t lvl0[3], lvl1[3], lvl2[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    for (int i_row_perm=0; i_row_perm < 6; i_row_perm++)
    {
        apply_row_permutation(lvl1, lvl0, perm_triple[i_row_perm]);
        normalize_b1b2b3 (lvl2, lvl1);
        int catalog_idx = search_catalog_entry(lvl2[1], lvl2[2]);
        assert( catalog_idx >=0 ); // assure entry was found
        catalog_entry_t *p_classmate = get_class_representative(catalog[catalog_idx]);
        assert( p_classmate->p_class == p_classmate || p_classmate->p_class == p_class_representative  );
        p_classmate->p_class = p_class_representative;
    }
}

void span_class_from_one_entry_third_level(catalog_entry_t *p_class)
{
    // checking for 2x2 squares
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    block_t lvl0[3], lvl1[3], lvl2[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    for (int i_row_1=0; i_row_1 < 3; i_row_1++)
    {
        for (int i_row_2=i_row_1+1; i_row_2 < 3; i_row_2++)
        {
            for (int i_col_1=0; i_col_1 < 9; i_col_1++)
            {
                for (int i_col_2=i_col_1+1; i_col_2 < 9; i_col_2++)
                {
                    candidate_t *p11 =  &lvl0[i_col_1 / 3][i_row_1][i_col_1 % 3];
                    candidate_t *p12 =  &lvl0[i_col_2 / 3][i_row_1][i_col_2 % 3];
                    candidate_t *p21 =  &lvl0[i_col_1 / 3][i_row_2][i_col_1 % 3];
                    candidate_t *p22 =  &lvl0[i_col_2 / 3][i_row_2][i_col_2 % 3];
                    if (*p11 == *p22 && *p12 == *p21)
                    {
                        swap (p11, p12);
                        swap (p21, p22);
                        copy_block(lvl1[0], lvl0[0]);
                        copy_block(lvl1[1], lvl0[1]);
                        copy_block(lvl1[2], lvl0[2]);
                        swap (p11, p12);
                        swap (p21, p22);
                        normalize_b1b2b3 (lvl2, lvl1);
                        int catalog_idx = search_catalog_entry(lvl2[1], lvl2[2]);
                        assert( catalog_idx >=0 ); // assure entry was found
                        catalog_entry_t *p_classmate = get_class_representative(catalog[catalog_idx]);
                        assert( p_classmate->p_class == p_classmate || p_classmate->p_class == p_class_representative  );
                        p_classmate->p_class = p_class_representative;
                    }
                }
            }
        }
    }
}

void span_class_from_one_entry_fourth_level(catalog_entry_t *p_class)
{
    // checking for 2x3 rectangles
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    block_t lvl0[3], lvl1[3], lvl2[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    for (int i_row_1=0; i_row_1 < 3; i_row_1++)
    {
        for (int i_row_2=i_row_1+1; i_row_2 < 3; i_row_2++)
        {
            for (int i_col_1=0; i_col_1 < 9; i_col_1++)
            {
                for (int i_col_2=i_col_1+1; i_col_2 < 9; i_col_2++)
                {
                    for (int i_col_3=i_col_2+1; i_col_3 < 9; i_col_3++)
                    {
                        candidate_t *p_src[2][3] =  
                          { { &lvl0[i_col_1 / 3][i_row_1][i_col_1 % 3], 
                              &lvl0[i_col_2 / 3][i_row_1][i_col_2 % 3], 
                              &lvl0[i_col_3 / 3][i_row_1][i_col_3 % 3] },
                            { &lvl0[i_col_1 / 3][i_row_2][i_col_1 % 3], 
                              &lvl0[i_col_2 / 3][i_row_2][i_col_2 % 3], 
                              &lvl0[i_col_3 / 3][i_row_2][i_col_3 % 3] } };
                        candidates_t cand_row1 = 0;
                        candidates_t cand_row2 = 0;
                        SET(cand_row1, *p_src[0][0]);
                        SET(cand_row1, *p_src[0][1]);
                        SET(cand_row1, *p_src[0][2]);
                        SET(cand_row2, *p_src[1][0]);
                        SET(cand_row2, *p_src[1][1]);
                        SET(cand_row2, *p_src[1][2]);
                        if (cand_row1 == cand_row2)  // the two triples from same three columns of two rows are equal setwise
                        {
                            candidate_t *p_dest[2][3] =
                              { { &lvl1[i_col_1 / 3][i_row_1][i_col_1 % 3], 
                                  &lvl1[i_col_2 / 3][i_row_1][i_col_2 % 3], 
                                  &lvl1[i_col_3 / 3][i_row_1][i_col_3 % 3] },
                                { &lvl1[i_col_1 / 3][i_row_2][i_col_1 % 3], 
                                  &lvl1[i_col_2 / 3][i_row_2][i_col_2 % 3], 
                                  &lvl1[i_col_3 / 3][i_row_2][i_col_3 % 3] } };
                            copy_block(lvl1[0], lvl0[0]);
                            copy_block(lvl1[1], lvl0[1]);
                            copy_block(lvl1[2], lvl0[2]);
                             *p_dest[0][0] = *p_src[1][0];
                             *p_dest[0][1] = *p_src[1][1];
                             *p_dest[0][2] = *p_src[1][2];
                             *p_dest[1][0] = *p_src[0][0];
                             *p_dest[1][1] = *p_src[0][1];
                             *p_dest[1][2] = *p_src[0][2];
                            normalize_b1b2b3 (lvl2, lvl1);
                            int catalog_idx = search_catalog_entry(lvl2[1], lvl2[2]);
                            assert( catalog_idx >=0 );
                            catalog_entry_t *p_classmate = get_class_representative(catalog[catalog_idx]);
                            assert( p_classmate->p_class == p_classmate || p_classmate->p_class == p_class_representative  );
                            p_classmate->p_class = p_class_representative;
                        }
                    }
                }
            }
        }
    }
}

void span_class_from_one_entry_fifth_level(catalog_entry_t *p_class)
{
    // checking for 2x4 rectangles
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    block_t lvl0[3], lvl1[3], lvl2[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    for (int i_row_1=0; i_row_1 < 3; i_row_1++)
    {
        for (int i_row_2=i_row_1+1; i_row_2 < 3; i_row_2++)
        {
            for (int i_col_1=0; i_col_1 < 9; i_col_1++)
            {
                for (int i_col_2=i_col_1+1; i_col_2 < 9; i_col_2++)
                {
                    for (int i_col_3=i_col_2+1; i_col_3 < 9; i_col_3++)
                    {
                        for (int i_col_4=i_col_3+1; i_col_4 < 9; i_col_4++)
                        {
                            candidate_t *p_src[2][4] =  
                            { { &lvl0[i_col_1 / 3][i_row_1][i_col_1 % 3], 
                                &lvl0[i_col_2 / 3][i_row_1][i_col_2 % 3], 
                                &lvl0[i_col_3 / 3][i_row_1][i_col_3 % 3],
                                &lvl0[i_col_4 / 3][i_row_1][i_col_4 % 3] },
                              { &lvl0[i_col_1 / 3][i_row_2][i_col_1 % 3], 
                                &lvl0[i_col_2 / 3][i_row_2][i_col_2 % 3], 
                                &lvl0[i_col_3 / 3][i_row_2][i_col_3 % 3], 
                                &lvl0[i_col_4 / 3][i_row_2][i_col_4 % 3] } };
                            candidates_t cand_row1 = 0;
                            candidates_t cand_row2 = 0;
                            SET(cand_row1, *p_src[0][0]);
                            SET(cand_row1, *p_src[0][1]);
                            SET(cand_row1, *p_src[0][2]);
                            SET(cand_row1, *p_src[0][3]);
                            SET(cand_row2, *p_src[1][0]);
                            SET(cand_row2, *p_src[1][1]);
                            SET(cand_row2, *p_src[1][2]);
                            SET(cand_row2, *p_src[1][3]);
                            if (cand_row1 == cand_row2)  // the two quartetts from same four columns of two rows are equal setwise
                            {
                                candidate_t *p_dest[2][4] =
                                { { &lvl1[i_col_1 / 3][i_row_1][i_col_1 % 3], 
                                    &lvl1[i_col_2 / 3][i_row_1][i_col_2 % 3], 
                                    &lvl1[i_col_3 / 3][i_row_1][i_col_3 % 3], 
                                    &lvl1[i_col_4 / 3][i_row_1][i_col_4 % 3] },
                                  { &lvl1[i_col_1 / 3][i_row_2][i_col_1 % 3], 
                                    &lvl1[i_col_2 / 3][i_row_2][i_col_2 % 3], 
                                    &lvl1[i_col_3 / 3][i_row_2][i_col_3 % 3], 
                                    &lvl1[i_col_4 / 3][i_row_2][i_col_4 % 3] } };
                                copy_block(lvl1[0], lvl0[0]);
                                copy_block(lvl1[1], lvl0[1]);
                                copy_block(lvl1[2], lvl0[2]);
                                *p_dest[0][0] = *p_src[1][0];
                                *p_dest[0][1] = *p_src[1][1];
                                *p_dest[0][2] = *p_src[1][2];
                                *p_dest[0][3] = *p_src[1][3];
                                *p_dest[1][0] = *p_src[0][0];
                                *p_dest[1][1] = *p_src[0][1];
                                *p_dest[1][2] = *p_src[0][2];
                                *p_dest[1][3] = *p_src[0][3];
                                normalize_b1b2b3 (lvl2, lvl1);
                                int catalog_idx = search_catalog_entry(lvl2[1], lvl2[2]);
                                assert( catalog_idx >=0 );
                                catalog_entry_t *p_classmate = get_class_representative(catalog[catalog_idx]);
                                assert( p_classmate->p_class == p_classmate || p_classmate->p_class == p_class_representative  );
                                p_classmate->p_class = p_class_representative;
                            }
                        }
                    }
                }
            }
        }
    }
}

void span_class_from_one_entry_sixth_level(catalog_entry_t *p_class)
{
    // checking for 3x2 rectangles
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    block_t lvl0[3], lvl1[3], lvl2[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    for (int i_row_1=0; i_row_1 < 3; i_row_1++)
        for (int i_row_2=0; i_row_2 < 3; i_row_2++)
        {
            if (i_row_1 == i_row_2)
                continue;
            for (int i_row_3=0; i_row_3 < 3; i_row_3++)
            {
                if (i_row_1 == i_row_3 || i_row_2 == i_row_3)
                    continue;
                for (int i_col_1=0; i_col_1 < 9; i_col_1++)
                    for (int i_col_2=0; i_col_2 < 9; i_col_2++)
                    {
                        if (i_col_1 == i_col_2)
                            continue;

                        candidate_t *p_src[3][2] =  
                        { { &lvl0[i_col_1 / 3][i_row_1][i_col_1 % 3], 
                            &lvl0[i_col_2 / 3][i_row_1][i_col_2 % 3] },
                          { &lvl0[i_col_1 / 3][i_row_2][i_col_1 % 3], 
                            &lvl0[i_col_2 / 3][i_row_2][i_col_2 % 3] },
                          { &lvl0[i_col_1 / 3][i_row_3][i_col_1 % 3], 
                            &lvl0[i_col_2 / 3][i_row_3][i_col_2 % 3] },
                             };
                        candidates_t cand_col1 = 0;
                        candidates_t cand_col2 = 0;
                        SET(cand_col1, *p_src[0][0]);
                        SET(cand_col1, *p_src[1][0]);
                        SET(cand_col1, *p_src[2][0]);
                        SET(cand_col2, *p_src[0][1]);
                        SET(cand_col2, *p_src[1][1]);
                        SET(cand_col2, *p_src[2][1]);
                        if (cand_col1 == cand_col2)  // the two tripless from same three rows of two columns are equal setwise
                        {
                            candidate_t *p_dest[3][2] =
                            { { &lvl1[i_col_1 / 3][i_row_1][i_col_1 % 3], 
                                &lvl1[i_col_2 / 3][i_row_1][i_col_2 % 3] },
                              { &lvl1[i_col_1 / 3][i_row_2][i_col_1 % 3], 
                                &lvl1[i_col_2 / 3][i_row_2][i_col_2 % 3] },
                              { &lvl1[i_col_1 / 3][i_row_3][i_col_1 % 3], 
                                &lvl1[i_col_2 / 3][i_row_3][i_col_2 % 3] } };
                            copy_block(lvl1[0], lvl0[0]);
                            copy_block(lvl1[1], lvl0[1]);
                            copy_block(lvl1[2], lvl0[2]);
                            *p_dest[0][0] = *p_src[0][1];
                            *p_dest[1][0] = *p_src[1][1];
                            *p_dest[2][0] = *p_src[2][1];
                            *p_dest[0][1] = *p_src[0][0];
                            *p_dest[1][1] = *p_src[1][0];
                            *p_dest[2][1] = *p_src[2][0];
                            normalize_b1b2b3 (lvl2, lvl1);
                            int catalog_idx = search_catalog_entry(lvl2[1], lvl2[2]);
                            assert( catalog_idx >=0 );
                            catalog_entry_t *p_classmate = get_class_representative(catalog[catalog_idx]);
                            assert( p_classmate->p_class == p_classmate || p_classmate->p_class == p_class_representative  );
                            p_classmate->p_class = p_class_representative;
                        }
                    }
                }
        }
}

void span_class_from_one_entry_seventh_level_old(catalog_entry_t *p_class)
{
    catalog_entry_t *p_class_representative = get_class_representative(p_class);
    assert( p_class_representative->p_class == p_class_representative );

    block_t lvl0[3], lvl1[3], lvl2[3];

    init_b1_standard (lvl0[0]);
    copy_block (lvl0[1], p_class->b2);
    copy_block (lvl0[2], p_class->b3);

    candidates_t rowsets[3] = { 0, 0, 0};
    int state [9] = { 0, 0 , 0, 0, 0, 0, 0, 0, 0 };
    int i_state = 0;
    int count = 0;
    while (i_state >= 0)
    {
        int box = i_state / 3;
        int col = i_state % 3;
        candidate_t row0 = lvl0[box][perm_triple[state[i_state]][0]][col];
        candidate_t row1 = lvl0[box][perm_triple[state[i_state]][1]][col];
        candidate_t row2 = lvl0[box][perm_triple[state[i_state]][2]][col];
        if (ISNOTSET(rowsets[0],row0) && ISNOTSET(rowsets[1],row1) && ISNOTSET(rowsets[2],row2))
        {
            lvl1[box][0][col] = row0;
            lvl1[box][1][col] = row1;
            lvl1[box][2][col] = row2;
            SET(rowsets[0],row0);
            SET(rowsets[1],row1);
            SET(rowsets[2],row2);
            i_state++;
            if (i_state == 9)
            {
                // success, found valid permutation
                // TODO: ...
                count ++; 
                i_state--;
                int box = i_state / 3;
                int col = i_state % 3;
                RESET(rowsets[0], lvl1[box][0][col]);
                RESET(rowsets[1], lvl1[box][1][col]);
                RESET(rowsets[2], lvl1[box][2][col]);
                state[i_state]++;
            }
        }
        else {
            state[i_state]++;
        }
        while (i_state >=0 && state[i_state] == 6)
        {
            state[i_state] = 0;
            i_state--;
        }
    }
    printf("level 7: %d\n", count);
}

namespace level7 {

catalog_entry_t *p_class;
catalog_entry_t *p_class_representative;
block_t lvl0[3], lvl1[3], lvl2[3];
candidates_t rowsets[3] = { 0, 0, 0};
int state [9] = { 0, 0 , 0, 0, 0, 0, 0, 0, 0 };
int count = 0;

void span_col_seventh_level(int i_state)
{
    if (i_state == 9)
    {
        // configuration found
        count++;
        normalize_b1b2b3 (lvl2, lvl1);
        int catalog_idx = search_catalog_entry(lvl2[1], lvl2[2]);
        assert( catalog_idx >=0 );
        catalog_entry_t *p_classmate = get_class_representative(catalog[catalog_idx]);
        assert( p_classmate->p_class == p_classmate || p_classmate->p_class == p_class_representative  );
        p_classmate->p_class = p_class_representative;
    }
    else {
        int box = (i_state) / 3;
        int col = (i_state) % 3;
        for (int i_perm=0; i_perm < 6; i_perm++)
        {
            candidate_t row0 = lvl0[box][perm_triple[i_perm][0]][col];
            candidate_t row1 = lvl0[box][perm_triple[i_perm][1]][col];
            candidate_t row2 = lvl0[box][perm_triple[i_perm][2]][col];
            if (ISNOTSET(rowsets[0],row0) && ISNOTSET(rowsets[1],row1) && ISNOTSET(rowsets[2],row2))
            {
                lvl1[box][0][col] = row0;
                lvl1[box][1][col] = row1;
                lvl1[box][2][col] = row2;
                SET(rowsets[0],row0);
                SET(rowsets[1],row1);
                SET(rowsets[2],row2);
                span_col_seventh_level(i_state+1);
                RESET(rowsets[0],row0);
                RESET(rowsets[1],row1);
                RESET(rowsets[2],row2);
            }
        }
    }
}

}

void span_class_from_one_entry_seventh_level(catalog_entry_t *p_class)
{
    level7::p_class = p_class;
    level7::p_class_representative = get_class_representative(p_class);
    assert( level7::p_class_representative->p_class == level7::p_class_representative );

    init_b1_standard (level7::lvl0[0]);
    copy_block (level7::lvl0[1], p_class->b2);
    copy_block (level7::lvl0[2], p_class->b3);

    level7::count = 0;
    level7::span_col_seventh_level(0);
//    printf("level 7: %d\n", level7::count);
}

int span_classes(span_class_from_one_entry_t span_class_from_one_entry)
{
    for (int idx=0; idx < catalog_size; idx++)
        span_class_from_one_entry(catalog[idx]);
    int no_of_classes = get_no_of_classes();
    return no_of_classes;
}

int get_class_size_distribution(int **outbuf)
{
    int *class_size = new int[catalog_size];
    for (int idx=0; idx < catalog_size; idx++)
        class_size[idx] = 0;
    for (int idx=0; idx < catalog_size; idx++)
        class_size[get_class_representative(catalog[idx])->id]++;
    int max_class_size = 0; // get_max_class_size()
    for (int idx=0; idx < catalog_size; idx++)
        if (class_size[idx] > max_class_size)
            max_class_size = class_size[idx];
    int *class_size_distribution = new int [max_class_size+1];
    for (int idx=0; idx <= max_class_size; idx++)
        class_size_distribution[idx] = 0;
    for (int idx=0; idx < catalog_size; idx++)
        class_size_distribution[class_size[idx]]++;
    *outbuf = class_size_distribution;
    delete [] class_size;
    return max_class_size;
}


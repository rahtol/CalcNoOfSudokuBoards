
#include <stdio.h>
#include "sudoku-count.h"


candidate_t grid [][9] = {
    { 1, 2, 3, 4, 7, 8, 5, 6, 9},
    { 4, 5, 6, 1, 3, 9, 2, 7, 8},
    { 7, 8, 9, 2, 5, 6, 1, 3, 4},
    { 2, 0, 0, 0, 0, 0, 0, 0, 0},
    { 3, 0, 0, 0, 0, 0, 0, 0, 0},
    { 5, 0, 0, 0, 0, 0, 0, 0, 0},
    { 6, 0, 0, 0, 0, 0, 0, 0, 0},
    { 8, 0, 0, 0, 0, 0, 0, 0, 0},
    { 9, 0, 0, 0, 0, 0, 0, 0, 0}
};

candidates_t col_candidates [9];  
candidates_t row_candidates [9];  
candidates_t box_candidates [9];  
candidate_t FIRST [1024];

void init_FIRST()
{
    for (int i=0; i<1024; i++)
    {
        FIRST [i] = 0;
        for (candidate_t n=1; n<=9; n++)
        {
            if ((i & (1<<n)) != 0)
            {
                FIRST [i] = n;
                break;
            }
        } 
    }
}

void init_candidate_lists()
{
    candidates_t ALL_CANDIDATES = 0;
    for (candidate_t n=1; n<=9; n++)
        SET(ALL_CANDIDATES, n);

    for (int i=0; i<9; i++)
    {
        row_candidates [i] = ALL_CANDIDATES;
        col_candidates [i] = ALL_CANDIDATES;
        box_candidates [i] = ALL_CANDIDATES;
    }

    for (int row=0; row<9; row++)
        for(int col=0; col<9; col++)
            if (grid [row][col] != 0)
            {
                int box = (row / 3) * 3 + (col / 3);
                RESET(row_candidates [row], grid [row][col]);
                RESET(col_candidates [col], grid [row][col]);
                RESET(box_candidates [box], grid [row][col]);
            }
}

struct backtrace_stack_entry_t {
    int row;
    int col;
    candidates_t candidates;
} backtrace_stack [81];

sudoku_count_t calc_no_soduko_sub0_iter (int row, int col)
{
    sudoku_count_t count = 0;
    struct backtrace_stack_entry_t *p_cur = backtrace_stack;
    struct backtrace_stack_entry_t *p_next = backtrace_stack;

    p_next->row = 3;
    p_next->col = 1;
    p_next->candidates = col_candidates [1] & row_candidates [3] & box_candidates [3];
    p_cur = p_next++;  // push operation
 
    while (true) // while stack not  empty via break below
    {
        if (p_cur->candidates == 0)
        {
            // pop operation
            p_next = p_cur;
            if (p_next == backtrace_stack)
                break; // stack is empty, terminate
            p_cur--;
            candidate_t n = grid[p_cur->row][p_cur->col];
            grid[p_cur->row][p_cur->col] = 0;
            int box = (p_cur->row / 3) * 3 + (p_cur->col / 3);
            SET(row_candidates [p_cur->row], n);
            SET(col_candidates [p_cur->col], n);
            SET(box_candidates [box], n);
        }
        else 
        {
            candidate_t n = FIRST [p_cur->candidates];
            RESET(p_cur->candidates, n);
            grid [p_cur->row][p_cur->col] = n;
            int box = (p_cur->row / 3) * 3 + (p_cur->col / 3);
            RESET(row_candidates [p_cur->row], n);
            RESET(col_candidates [p_cur->col], n);
            RESET(box_candidates [box], n);
            if (p_cur->row == 8 && p_cur->col == 8)
            {
                // complete
                count++;
                p_cur = p_next++; // pseudo-push
                p_cur->candidates = 0;
            }
            else
            {
                // push operation, i.e. advance to next cell
                p_next->row = p_cur->row + p_cur->col / 8;
                p_next->col = (p_cur->col == 8 ? 1 : p_cur->col + 1);
                int box = (p_next->row / 3) * 3 + (p_next->col / 3);
                p_next->candidates = col_candidates [p_next->col] & row_candidates [p_next->row] & box_candidates [box];
                p_cur = p_next++; // push
            }
        }
    }
    return count;
}

sudoku_count_t calc_no_soduko_sub0 (int row, int col)
{
    sudoku_count_t count = 0;
    int box = (row / 3) * 3 + (col / 3);
    candidates_t candidates = col_candidates [col] & row_candidates [row] & box_candidates [box];
    candidate_t n;
    while ((n = FIRST [candidates]) != 0)
    {
        RESET(candidates, n);
        if (row == 8 && col == 8)
        {
            grid [row][col] = n;
            count++;
            grid [row][col] = 0;
        }
        else {
            RESET(row_candidates [row], n);
            RESET(col_candidates [col], n);
            RESET(box_candidates [box], n);
            grid [row][col] = n;
 
            count += calc_no_soduko_sub0(row + col / 8, (col == 8 ? 1 : col + 1));
 
            grid [row][col] = 0;
            SET(row_candidates [row], n);
            SET(col_candidates [col], n);
            SET(box_candidates [box], n);
        }
    }

    return count;
}

sudoku_count_t calc_no_soduko_sub1 (block_t b2, block_t b3)
{
    sudoku_count_t count = 0;

    init_FIRST();

    // copy b2 and b3 into grid
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
        {
            grid [i][j+3] = b2[i][j];
            grid [i][j+6] = b3[i][j];
        }

    // possible values for first column of b4 and b7 taking into account that b1 is in standard form
    candidate_t l [] = {2, 3, 5, 6, 8, 9};

    // exhaust all possibilitiers to fill first colum of b4 and b7 
    // with candidates from list l in lexicographical order
    grid [3][0] = l [0];
    for (int i1=1; i1<5; i1++)
    {
        grid [4][0] = l [i1];
        for (int i2=i1+1; i2<6; i2++)
        {
            grid [5][0] = l [i2];
            int row = 6;
            for (int i=0; i<6; i++)
            {
                if ((l[i] != grid [3][0]) && (l[i] != grid [4][0]) && (l[i] != grid [5][0]))
                {
                    grid [row][0] = l[i];
                    row++;
                }
            }
//            printf("%d %d %d %d %d %d ", grid [3][0], grid [4][0], grid [5][0], grid [6][0], grid [7][0], grid [8][0]);
            init_candidate_lists();
            count += calc_no_soduko_sub0 (3, 1);
//            printf(" count=%ld\n", count);
        }
    }
    return count;
}

#ifdef TEST_SUDOKU_2
#include <chrono>
block_t b2 = {
    { 4, 7, 8 },
    { 1, 3, 9 },
    { 2, 5, 6 }
};

block_t b3 = {
    { 5, 6, 9 },
    { 2, 7, 8 },
    { 1, 3, 4 }
};

int main()
{
    printf("Counting Sudokus.\n");


    auto t1 = std::chrono::high_resolution_clock::now();
    sudoku_count_t no = calc_no_soduko_sub1 (b2, b3);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto dt1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    printf("####  %5dms  ####\n", (int) dt1.count());

    printf("no=%ld\n", no);
}

#endif
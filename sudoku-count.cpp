#include <stdio.h>
#include "sudoku-count.h"
#include <assert.h>
#include <chrono>

// How many different Sudoku solutions, i.e. complete grids obeying the Sudoku constraints, are there ?
// This program reimplements the algorithm outlined in:
//      Mathematics of Sudoku I
//      Bertram Felgenhauer, Frazer Jarvis
//      January 25, 2006
// File SudokuNbGrilles.pdf


void print_class_size_distribution(int no_classes)
{
    int *p_class_size_distribution = NULL;
    int max_class_size = get_class_size_distribution(&p_class_size_distribution);

    int no_catalog_entries = 0;
    for (int i=0; i <= max_class_size; i++)
    {
        no_catalog_entries += p_class_size_distribution[i];
        if (p_class_size_distribution[i] != 0)
            printf("class_size=%3d: no_classes: %5d\n", i, p_class_size_distribution[i]);
    }
    assert( no_catalog_entries == catalog_size );
    assert( no_classes == catalog_size - p_class_size_distribution[0] );

    printf("catalog_size=%d, no_classes=%d\n", catalog_size, no_classes);
}


sudoku_count_t calc_no_sudoku()
{
    sudoku_count_t count = 0;
    int no = 0;
    printf("Start backtracking.\n");
    for (int i=0; i < catalog_size; i++)
        if (is_class_representative(catalog[i]))
        {
            no++;
            auto t0 = std::chrono::high_resolution_clock::now();

            int class_size = get_class_size(catalog[i]);
            sudoku_count_t sub_count = calc_no_soduko_sub1(catalog[i]->b2, catalog[i]->b3);
            count += class_size * sub_count; 

            auto t1 = std::chrono::high_resolution_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
            printf("####  %2d:  %12lld  %8d  %5dms  ####\n", no, sub_count, class_size, (int) dt.count());

        }
    return count;
}


void main()
{
    printf("sudoku-count #108\n");

    // following allowing the algorithmus drawn in chapter 2
    // generate a catalog of 35288 normalzied entries
    auto t1 = std::chrono::high_resolution_clock::now();
    generate_catalog();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto dt1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    printf("####  0:  %5d  %5dms  ####\n", catalog_size, (int) dt1.count());

    // now reduce the number of relevant entries by combining them into 
    // equivalence classes so that all members of a class have same number
    // of completions to a full grid. each class has one representative.
    // combing is done in seven reduction levels as described in the article

    // level1: permuting the blocks b1-b3 and the colums within each of these blocks
    int no_classes_lvl1 = span_classes(span_class_from_one_entry_first_level);
    // expecting: no_classes_lvl1 == 2051
    auto t3 = std::chrono::high_resolution_clock::now();
    auto dt2 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2);
    printf("####  1:  %5d  %5dms  ####\n", no_classes_lvl1, (int) dt2.count());

    // level2: permuting the rows of b1, b2 and b3 simultaneously
    int no_classes_lvl2 = span_classes(span_class_from_one_entry_second_level);
    // expecting: no_classes_lvl2 == 416
    auto t4 = std::chrono::high_resolution_clock::now();
    auto dt3 = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3);
    printf("####  2:  %5d  %5dms  ####\n", no_classes_lvl2, (int) dt3.count());

    // level3: 2x2 squares
    int no_classes_lvl3 = span_classes(span_class_from_one_entry_third_level);
    // expecting: no_classes_lvl3 == 174
    auto t5 = std::chrono::high_resolution_clock::now();
    auto dt4 = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4);
    printf("####  3:  %5d  %5dms  ####\n", no_classes_lvl3, (int) dt4.count());

    // level4: 2x3 rectangles
    int no_classes_lvl4 = span_classes(span_class_from_one_entry_fourth_level);
    auto t6 = std::chrono::high_resolution_clock::now();
    auto dt5 = std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5);
    printf("####  4:  %5d  %5dms  ####\n", no_classes_lvl4, (int) dt5.count());

    // level5: 2x4 rectangles
    int no_classes_lvl5 = span_classes(span_class_from_one_entry_fifth_level);
    auto t7 = std::chrono::high_resolution_clock::now();
    auto dt6 = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6);
    printf("####  5:  %5d  %5dms  ####\n", no_classes_lvl5, (int) dt6.count());

    // level6: 4x2 rectangles
    int no_classes_lvl6 = span_classes(span_class_from_one_entry_sixth_level);
    // expecting: no_classes_lvl6 == 71
    auto t8 = std::chrono::high_resolution_clock::now();
    auto dt7 = std::chrono::duration_cast<std::chrono::milliseconds>(t8 - t7);
    printf("####  6:  %5d  %5dms  ####\n", no_classes_lvl6, (int) dt7.count());

    // level7: check all valid configurations of b1-b3 where the columns
    //         of the two configurations consist of the same numbers, in any order
    int no_classes_lvl7 = span_classes(span_class_from_one_entry_seventh_level);
    // expecting: no_classes_lvl7 == 44
    auto t9 = std::chrono::high_resolution_clock::now();
    auto dt8 = std::chrono::duration_cast<std::chrono::milliseconds>(t9 - t8);
    printf("####  7:  %5d  %5dms  ####\n", no_classes_lvl7, (int) dt8.count());

    // the reduction level sequence and the sequence of class numbers 
    // after each level is in strict accordance with the article

    // just dump some info on the final class size distribution,
    // i.e. how many classes of which size are there
    // the entry with class_size=0 counts the number of entries that are not 
    // a class representative so that the overall sum is the catalog_size 36288
    print_class_size_distribution(no_classes_lvl7);

    // TEST - Chapter 3.1, first paragraph states a class size of 178848/72=2484
    //        for the example element from the catalog below
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
    int catalog_idx = search_catalog_entry(b2, b3);
    int class_size = get_class_size(catalog[catalog_idx]);
    // expecting: class size=2484
    printf("class-id: %d, class_size=%d\n", catalog_idx, class_size);

    // finally do the brute force part according to chapter 3 
    // for each of the remaing 44 class representatives - takes a while
    sudoku_count_t total_count = calc_no_sudoku();
    printf("####  total_count=%lld  ####\n", total_count);
    // expecting: total_count = 3546146300288
    // this is the number of sudokus reduced by the symmetries used
    // final result is: total_count * 72 * 72 * 9!
    // not printing it here, because it does violate the limit of the 64-bit 
    // integer arithmetic used in this program 

}

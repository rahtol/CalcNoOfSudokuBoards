typedef unsigned candidate_t;  // candidate value in the range 1..9
typedef unsigned long long sudoku_count_t; // sudoku count value
typedef candidate_t block_t [3][3];

typedef unsigned candidates_t;  // set of candidate values represented as bitlist
#define SET(CANDIDATES,N) ((CANDIDATES) |= (1<<(N)))
#define RESET(CANDIDATES,N) ((CANDIDATES) &= ~(1<<(N)))
#define ISNOTSET(CANDIDATES,N) (((CANDIDATES) & (1<<(N))) == 0)

struct catalog_entry_t {
    int id;
    block_t b2;
    block_t b3;
    catalog_entry_t *p_class; // NULL for an class-represetative, other wise pointer to class-respresentative
} ;

typedef void span_class_from_one_entry_t (catalog_entry_t *catalog_entry);

extern catalog_entry_t *catalog[36288];
extern int catalog_size;

extern int perm_abc[3][3];
extern int perm_triple [6][3];


void generate_catalog();
int search_catalog_entry(block_t b2, block_t b3);
int get_class_size(catalog_entry_t *catalog_entry);
bool is_class_representative(catalog_entry_t *catalog_entry);
catalog_entry_t *get_class_representative(catalog_entry_t *catalog_entry);

int span_classes(span_class_from_one_entry_t span_class_from_one_entry);
void span_class_from_one_entry_first_level(catalog_entry_t *p_class_representative);
void span_class_from_one_entry_second_level(catalog_entry_t *p_class_representative);
void span_class_from_one_entry_third_level(catalog_entry_t *p_class_representative);
void span_class_from_one_entry_fourth_level(catalog_entry_t *p_class_representative);
void span_class_from_one_entry_fifth_level(catalog_entry_t *p_class_representative);
void span_class_from_one_entry_sixth_level(catalog_entry_t *p_class_representative);
void span_class_from_one_entry_seventh_level(catalog_entry_t *p_class_representative);
int get_class_size_distribution(int **outbuf);
sudoku_count_t calc_no_soduko_sub1 (block_t b2, block_t b3);

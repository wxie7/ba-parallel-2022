#pragma GCC optimize("Ofast")
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/sysinfo.h>

#define MAX_PRODUCTION2_NUM 512  /* 推出非终结符的产生式的数量 */
#define MAX_PRODUCTION1_NUM 128  /* 推出终结符的产生式的数量 */
#define MAX_VN_NUM          128  /* 最大的非终结符数量 */
#define MAX_VT_NUM          128  /* 最大的终结符的数量 */
#define MAX_STRING_LENGTH   1024 /* 待判断的字符串的最大长度 */
#define MAX_THREADS         64   /* 最大线程数量 */

struct binary_production {
    int parent;
    int left, right;
};
struct unary_production {
    int parent;
    unsigned char ch;
};
struct sector {
    int start;
    int end;
};

int vn_number;
int binary_production_number;
int unary_production_number;
int s_len;
char s[MAX_STRING_LENGTH];
int binaries_parent[MAX_PRODUCTION2_NUM];
/* int binaries_left[MAX_PRODUCTION2_NUM]; */
/* int binaries_right[MAX_PRODUCTION2_NUM]; */
struct binary_production binaries[MAX_PRODUCTION2_NUM];
struct unary_production unaries[MAX_PRODUCTION1_NUM];
struct sector vn_index[MAX_VN_NUM][MAX_VN_NUM];
struct sector vt_index[MAX_VT_NUM];
unsigned table_num[MAX_STRING_LENGTH][MAX_STRING_LENGTH][MAX_VN_NUM];
int table_list[MAX_STRING_LENGTH][MAX_STRING_LENGTH][MAX_VN_NUM + 1];

pthread_t threads[MAX_STRING_LENGTH];
sem_t sems[MAX_STRING_LENGTH];
pthread_mutex_t mutex;
int used_buf;


void input(void );
void initialize_table(void );
void init_sem(void );
void *algo_main_body(void *aux);

void *routine(void *aux);
void *base_routine(void *aux);

int main() {
    input();
    initialize_table();
    init_sem();
    algo_main_body(NULL);
    pthread_join(threads[s_len - 1], NULL);
    printf("%u\n", table_num[0][s_len - 1][0]);
    return 0;
}

void input(void ) {
    static char *input_filename = "input.txt";
    freopen(input_filename, "r", stdin);
    scanf("%d\n", &vn_number);
    scanf("%d\n", &binary_production_number);
    int i;
    for (i = 0; i < binary_production_number; i++)
        scanf("<%d>::=<%d><%d>\n",
              &binaries[i].parent,
              &binaries[i].left,
              &binaries[i].right);
    scanf("%d\n", &unary_production_number);
    for (i = 0; i < unary_production_number; i++)
        scanf("<%d>::=%c\n", &unaries[i].parent, &unaries[i].ch);
    scanf("%d\n", &s_len);
    scanf("%s\n", s);
}

int cmp_binary(const void *_a, const void *_b) {
    const struct binary_production *a =
            (const struct binary_production *)_a;
    const struct binary_production *b =
            (const struct binary_production *)_b;
    return a->left == b->left ?
                              (a->right == b->right ? a->parent -
                                                              b->parent :
                                                    a->right - b->right)
                              : a->left - b->left;
}
int cmp_unary(const void *_a, const void *_b) {
    const struct unary_production *a =
            (const struct unary_production *)_a;
    const struct unary_production *b =
            (const struct unary_production *)_b;
    return a->ch == b->ch ? (a->parent - b->parent) :
                          (a->ch < b->ch ? -1 : 1);
}
void initialize_table(void ) {
    int i, j;
    int left, right;
    unsigned char ch;
    int start, end;
    int parent;
    /* 初始化索引表 */
    qsort(unaries, unary_production_number,
          sizeof(struct unary_production),
          cmp_unary);
    qsort(binaries, binary_production_number,
          sizeof(struct binary_production),
          cmp_binary);

    for (i = 0; i < binary_production_number; ++i) {
        binaries_parent[i] = binaries[i].parent;
    }

    memset(vt_index, 0xff, sizeof(vt_index[0]) * MAX_VT_NUM);
    for (i = 0; i < unary_production_number; ++i) {
        ch = unaries[i].ch;
        if (vt_index[ch].start == -1) {
            vt_index[ch].start = vt_index[ch].end = i;
        }
        vt_index[ch].end += 1;
    }
    for (i = 0; i < vn_number; ++i) {
        memset(vn_index[i], 0xff, sizeof(vn_index[0][0]) * vn_number);
    }
    for (i = 0; i < binary_production_number; ++i) {
        left = binaries[i].left;
        right = binaries[i].right;
        if (vn_index[left][right].start == -1) {
            vn_index[left][right].start = vn_index[left][right].end = i;
        }
        vn_index[left][right].end += 1;
    }
    for (i = 0; i < s_len; ++i) {
        ch = s[i];
        start = vt_index[ch].start;
        end = vt_index[ch].end;
        for (j = start; j < end; ++j) {
            parent = unaries[j].parent;
            if (!table_num[i][i][parent]) {
                table_list[i][i][0]++;
                table_list[i][i][table_list[i][i][0]] = parent;
            }
            table_num[i][i][parent]++;
        }
    }
}

void init_sem(void ) {
    int i;
    for (i = 1; i < s_len; ++i) {
        sem_init(&sems[i], 0, 0);
    }
}

unsigned BC_buf[MAX_STRING_LENGTH][MAX_VN_NUM][MAX_VN_NUM];
int BC_count[MAX_STRING_LENGTH];
int BC_list[MAX_STRING_LENGTH][MAX_VN_NUM * MAX_VN_NUM][2];

/* optimize for sparse list */
void sub_str_process_v1(int i, int j, void *aux) {
    int k;
    int p, q;
    int p_range, q_range;
    int B, C, A;
    unsigned B_num, C_num;
    unsigned BC;
    int left, right;
    int binary_index;
    int thread_num = (int)aux;
    for (k = i; k <= j - 1; ++k) {
        p_range = table_list[i][k][0];
        q_range = table_list[k + 1][j][0];
        for (p = 1; p <= p_range; ++p) {
            for (q = 1; q <= q_range; ++q) {
                B = table_list[i][k][p];
                C = table_list[k + 1][j][q];

                if (vn_index[B][C].end != vn_index[B][C].start) {
                    BC = table_num[i][k][B] * table_num[k + 1][j][C];
                    left = vn_index[B][C].start;
                    right = vn_index[B][C].end;
                    for (binary_index = left; 
                            binary_index < right;
                            ++binary_index) {
                        A = binaries_parent[binary_index];
                        if (!table_num[i][j][A]) {
                            table_list[i][j][0]++;
                            table_list[i][j][table_list[i][j][0]] = A;
                        }
                        table_num[i][j][A] += BC;
                    }
                }
            }
        }
    }
}



void sub_str_process_v0(int i, int j, void *aux) {
    int k;
    int p, q;
    int p_range, q_range;
    int B, C, A;
    unsigned B_num, C_num;
    int left, right;
    int binary_index;
    int thread_num = (int)aux;
    memset(BC_buf + thread_num, 0, sizeof(BC_buf[0]));
    BC_count[thread_num] = 0;
    for (k = i; k <= j - 1; ++k) {
        p_range = table_list[i][k][0];
        q_range = table_list[k + 1][j][0];
        for (p = 1; p <= p_range; ++p) {
            for (q = 1; q <= q_range; ++q) {
                B = table_list[i][k][p];
                C = table_list[k + 1][j][q];
                B_num = table_num[i][k][B];
                C_num = table_num[k + 1][j][C];
                BC_buf[thread_num][B][C] += B_num * C_num;

            }
        }
    }
    for (B = 0; B < MAX_VT_NUM; ++B) {
        for (C = 0; C < MAX_VT_NUM; ++C) {
            if (vn_index[B][C].end != vn_index[B][C].start
                && BC_buf[thread_num][B][C] != 0) {
                left = vn_index[B][C].start;
                right = vn_index[B][C].end;
                for (binary_index = left; binary_index < right; ++binary_index) {
                    A = binaries_parent[binary_index];
                    if (!table_num[i][j][A]) {
                        table_list[i][j][0]++;
                        table_list[i][j][table_list[i][j][0]] = A;
                    }
                    table_num[i][j][A] += BC_buf[thread_num][B][C];
                }
            }
        }
    }
}


void *algo_main_body(void *aux) {
    int i;
    int sub_len;
    for (i = 1; i < s_len; ++i) {
        sem_post(&sems[1]);
    }
    for (i = 1; i < s_len; ++i) {
        pthread_create(&threads[i],
                       NULL,
                       routine,
                       (void *)i);
    }
    return NULL;
}

void *routine(void *aux) {
    int thread_num = (int)aux;
    int i;
    void (*f) (int, int, void *);
    // f = (thread_num < s_len / 4) ? sub_str_process_v1 : sub_str_process_v0;

    f = sub_str_process_v1;

    for (i = 0; i < s_len - thread_num; ++i) {
        sem_wait(&sems[thread_num]);
        f(i, i + thread_num, aux);
        if (i != 0 && thread_num != s_len - 1)
            sem_post(&sems[thread_num + 1]);
    }
    return NULL;
}

#pragma clang diagnostic pop


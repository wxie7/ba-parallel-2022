#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PRODUCTION2_NUM 512
#define MAX_PRODUCTION1_NUM 128
#define MAX_VN_NUM          128
#define MAX_VT_NUM          128
#define MAX_STRING_LENGTH   1024

struct Production2 {
	int parent;
	int child1;
	int child2;
} production2[MAX_PRODUCTION2_NUM];

struct Production1 {
	int parent;
	unsigned char child;
} production1[MAX_PRODUCTION1_NUM];

struct Sector {
	int begin;
	unsigned len;
};
struct Sector vnIndex[MAX_VN_NUM][MAX_VN_NUM];
struct Sector vtIndex[MAX_VT_NUM];
char s[MAX_STRING_LENGTH];

struct SubTree {
	int root;
	unsigned len;
} subTreeTable[MAX_STRING_LENGTH][MAX_STRING_LENGTH][MAX_VN_NUM];

int subTreeLenTable[MAX_STRING_LENGTH][MAX_STRING_LENGTH];
struct SubTree subTreeBuf[2][MAX_STRING_LENGTH];

int vn_num;
int production2_num;
int production1_num;
int s_len;

int i, j;
int _len;

int cmp_production1(const void *, const void *);
int cmp_production2(const void *, const void *);

int main() {
	/* input data */
	freopen("../pat.txt", "r", stdin);
	scanf("%d\n", &vn_num);
	scanf("%d\n", &production2_num);
	for (i = 0; i < production2_num; i++)
		scanf("<%d>::=<%d><%d>\n",
			  &production2[i].parent,
			  &production2[i].child1,
			  &production2[i].child2);
	scanf("%d\n", &production1_num);
	for (i = 0; i < production1_num; i++)
		scanf("<%d>::=%c\n",
			  &production1[i].parent,
			  &production1[i].child);
	scanf("%d\n", &s_len);
	scanf("%s\n", s);

	/* preprocess data */
	qsort(production1,
		  production1_num,
		  sizeof(struct Production1),
		  cmp_production1);
	/* 所有可打印的ascii字符 */
	for (i = 0; i < MAX_VT_NUM; ++i) {
		vtIndex[i].begin = -1;
		vtIndex[i].len = 0;
	}
	for (i = 0; i < production1_num; ++i) {
		unsigned char ch = production1[i].child;
		if (vtIndex[ch].begin == -1) {
			vtIndex[ch].begin = i;
		}
		vtIndex[ch].len++;
	}
	/* initialize the table */
	for (i = 0; i < s_len; ++i) {
		unsigned char ch = s[i];
		int begin = vtIndex[ch].begin;
		int end = begin + vtIndex[ch].len;
		for (j = begin; j < end; ++j) {
			struct SubTree subTree = {
					.root = production1[j].parent,
					.len = 1,
			};
			subTreeTable[i][i][subTreeLenTable[i][i]++] = subTree;
		}
	}

	qsort(production2,
		  production2_num,
		  sizeof (struct Production2),
		  cmp_production2);
	for (i = 0; i < vn_num; ++i) {
		for (j = 0; j < vn_num; ++j) {
			vnIndex[i][j].begin = -1;
			vnIndex[i][j].len = 0;
		}
	}
	for (i = 0; i < production2_num; ++i) {
		int n1 = production2[i].child1;
		int n2 = production2[i].child2;
		if (vnIndex[n1][n2].begin == -1) {
			vnIndex[n1][n2].begin = i;
		}
		vnIndex[n1][n2].len++;
	}

	/* very very consuming */
	for (_len = 2; _len <= s_len; ++_len) {
		int left;
		for (left = 0; left <= s_len - _len; ++left) {
			int _curr = 0, _last = 1;
			int oldTreeLen = 0;
			int right;
			for (right = left + 1; right < left + _len; ++right) {
				int i1, i2;
				for (i1 = 0; i1 < subTreeLenTable[left][right - 1]; ++i1) {
					struct SubTree subTreeChild1 = subTreeTable[left][right - 1][i1];
					for (i2 = 0; i2 < subTreeLenTable[right][left + _len - 1]; ++i2) {
						struct SubTree subTreeChild2 = subTreeTable[right][left + _len - 1][i2];
						int begin = vnIndex[subTreeChild1.root][subTreeChild2.root].begin;
						int end = begin + vnIndex[subTreeChild1.root][subTreeChild2.root].len;
						if (begin == end) continue;
						// TODO
						//
						int _tmp = _last;
						_last = _curr, _curr = _tmp;
						int newTreeLen = 0;
						int k = 0;
						for (j = begin; j < end; ++j) {
							struct SubTree subTreeParent = {
									.root = production2[j].parent,
									.len = subTreeChild1.len * subTreeChild2.len,
							};
							while (k < oldTreeLen && subTreeParent.root > subTreeBuf[_last][k].root) {
								subTreeBuf[_curr][newTreeLen++] = subTreeBuf[_last][k++];
							}
							if (k < oldTreeLen && subTreeParent.root == subTreeBuf[_last][k].root) {
								subTreeParent.len += subTreeBuf[_last][k++].len;
							}
							subTreeBuf[_curr][newTreeLen++] = subTreeParent;
						}
						while (k < oldTreeLen) {
							subTreeBuf[_curr][newTreeLen++] = subTreeBuf[_last][k++];
						}
						oldTreeLen = newTreeLen;
					}
				}
			}
			subTreeLenTable[left][left + _len - 1] = oldTreeLen;
			if (subTreeLenTable[left][left + _len - 1] > 0) {
				memcpy(subTreeTable[left][left + _len - 1],
					   subTreeBuf[_curr],
					   subTreeLenTable[left][left + _len - 1]
							   * sizeof(struct SubTree));
			}
		}
	}
	unsigned ans = 0;
	if (subTreeLenTable[0][s_len - 1] > 0) {
		if (subTreeTable[0][s_len - 1][0].root == 0) {
			ans = subTreeTable[0][s_len - 1][0].len;
		}
	}
	printf("%u\n", ans);
	long pile = clock();
	printf("all time: %lu\n", pile / CLOCKS_PER_SEC);
	return 0;
}

int cmp_production1(const void *_a, const void *_b) {
	const struct Production1 *a = (struct Production1 *)_a;
	const struct Production1 *b = (struct Production1 *)_b;
	return a->child == b->child ? a->parent - b->parent : a->child - b->child;
}

int cmp_production2(const void *_a, const void *_b) {
	const struct Production2 *a = (struct Production2 *)_a;
	const struct Production2 *b = (struct Production2 *)_b;
	return a->child1 == b->child1 ?
		   (a->child2 == b->child2 ? a->parent - b->parent : a->child2 - b->child2) :
		   a->child1 - b->child1;
}
#pragma clang diagnostic pop
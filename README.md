# week07

## history

hpc使用cpu68，48核。

| version   | hpc   | local  | hpc   | local | note                   |
|:---------:|:-----:|:------:|:-----:|:-----:|:----------------------:|
| v0.1      | 60+?  |        | 30+?  |       | openmp优化单次斜对角线 |
| v0.2      | 2.778 | ...    | 1.437 | ...   | pthread线程池          |
| v0.3      | 2.341 |        | 1.098 |       | openmp，cache优化      |
| v0.35-rc1 | 2.63  | 10.364 | 1.208 | ...   | 取消冗余循环           |
|           |       |        |       |       |                        |
|           |       |        |       |       |                        |


## description

输入为Chomsky范式，输入字符串为`s[0..len]`，构造大小为`len * len`的表（只需填充一半）。

最初的cyk算法。
```c
// 表的初始化
for (i, ch in enumerate(s)) {
    if (存在 A-> ch) {
        将`A->ch` 填入 table(i, i)
    }
}
for (sublen from 2 to len) {
    for (left from 0 to len - sublen) {
        right = left + sublen - 1; // [left, right] 区间
        for (k from left to right - 1) {
            for (B in select(table(left, k))) {
                for (C in select(table(k + 1, right))) {
                    if (存在 A-> B C) {
                        将`A->B C` 填入table(left, right)
                    }
                }
            }
        }
    }
}
// Z为起始符号
if (Z in table(0, len - 1)) {
    Ok!
}
```
如果要同时计算生成树的数量，记`treeNumber(T)`为构成`T`的生成树的数量。
如果`A->BC`，那么`treeNumber(A)=treeNumber(B) * treeNumber(C)`.

流程如下：

```c
// 将每个产生式包装，赋予编号和数量（数量对应上文中的生成树数量）
// 表的初始化
for (i, ch in enumerate(s)) {
    // ch作索引，检索出A，但可能存在多个A
    // 一对多的映射
    if (exist A-> ch) {
        Wrap(A->ch).num = 1;
        fill Wrap(A->ch) in table(i, i)
    }
}
for (sublen from 2 to len) {
    for (left from 0 to len - sublen) {
        right = left + sublen - 1; // [left, right] 区间
        for (k from left to right - 1) {
            for (B in select(table(left, k))) {
                for (C in select(table(k + 1, right))) {
                    // 以B，C作索引，检索出A
                    if (exist A-> B C) {
                        Wrap(A->BC).num = Wrap(B).num * Wrap(C).num;
                        // 并查集
                        // 归并指标为A，即产生式左部
                        fill Wrap(A->BC) in table(left, right)
                        if table(left, right) exist multiple Wrap(A) {
                            merge Wrap(A).num by add;
                        }
                    }
                }
            }
        }
    }
}
// Z为起始符号
if (Z in table(0, len - 1)) {
    ans = table(0, len - 1).get(Z).num;
}
```
## optimization

结构体赋值改为指针复制。

如果使用并查集，合并树时会导致数据竞争，只有128个非终结符，直接使用数组替代，
表最大是1024*1024，表中的每个单元格都代表一个并查集，能在O(1)的时间验证，
但遍历集合中的元素不能达到O(size(set))，除非使用辅助空间。

位集合作并查，辅助空间作遍历。

辅助空间结构：

    -------------------------------------
    | number | val0 | val1 | val2 | ... |
    -------------------------------------

不需要位集合作并查集，直接用数量数组标识产生式左部是否已经存在。

需要的数据结构：
    1. 数量数组，标识产生式左部的数量。
    2. 辅助数组，对应上文select的遍历操作。

相同的产生式右部，可以对应多个左部，需要链表

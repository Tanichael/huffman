#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "encode.h"

#define NSYMBOLS 256

static int symbol_count[NSYMBOLS];

static int flag[NSYMBOLS];

// 以下このソースで有効なstatic関数のプロトタイプ宣言

// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// symbol_count をリセットする関数
static void reset_count(void);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right, char* temp);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数
static Node *build_tree(void);


// 以下 static関数の実装
static void count_symbols(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
	fprintf(stderr, "error: cannot open %s\n", filename);
	exit(1);
    }

    // 1Byteずつ読み込み、カウントする
    /*
      write a code for counting
    */

    int c;
    while((c = fgetc(fp)) != EOF) {
        symbol_count[c]++;
    }

    fclose(fp);
}
static void reset_count(void)
{
    for (int i = 0 ; i < NSYMBOLS ; i++) {
        symbol_count[i] = 0;
        flag[i] = 0;
    }

}

static Node *create_node(int symbol, int count, Node *left, Node *right, char *temp)
{
    Node *ret = (Node *)malloc(sizeof(Node));
    *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right, .code = temp};
    return ret;
}

static Node *pop_min(int *n, Node *nodep[])
{
    // Find the node with the smallest count
    // カウントが最小のノードを見つけてくる
    int argmin = 0;
    for (int i = 0; i < *n; i++) {
        if (nodep[i]->count < nodep[argmin]->count) {
            argmin = i;
        }
    }
    
    Node *node_min = nodep[argmin];
    
    // Remove the node pointer from nodep[]
    // 見つかったノード以降の配列を前につめていく
    for (int i = argmin; i < (*n) - 1; i++) {
	    nodep[i] = nodep[i + 1];
    }
    // 合計ノード数を一つ減らす
    (*n)--;
    
    return node_min;
}

static Node *build_tree(void)
{
    int n = 0;
    Node *nodep[NSYMBOLS];
    
    for (int i = 0; i < NSYMBOLS; i++) {
        // カウントの存在しなかったシンボルには何もしない
        if (symbol_count[i] == 0) continue;
        char *temp = "\0";
        nodep[n++] = create_node(i, symbol_count[i], NULL, NULL, temp);
    }

    const int dummy = -1; // ダミー用のsymbol を用意しておく
    while (n >= 2) {
        Node *node1 = pop_min(&n, nodep);
        Node *node2 = pop_min(&n, nodep);
        
        // Create a new node
        // 選ばれた2つのノードを元に統合ノードを新規作成
        // 作成したノードはnodep にどうすればよいか?
        char *temp = "\0";
        nodep[n++] = create_node(dummy, node1->count + node2->count, node1, node2, temp);
	
    }

    // なぜ以下のコードで木を返したことになるか少し考えてみよう
    return (n==0)?NULL:nodep[0];
}



// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
// 現状は何もしていない（再帰してたどっているだけ）
void traverse_tree(const int depth, Node *np, char* temp)
{	
    if (np == NULL) return;
    if(depth == 0) {
        printf(".\n");
        np->code[0] = '\0';
    } else {
        for(int i = 0; i < depth - 1; i++) {
            int key = 1;
            for(int j = 0; j < i + 1; j++) {
                key *= 2;
            }
            if(flag[i+1] < key && flag[i+1] % 2 != 0) {
                printf("│   ");
            } else {
                printf("    ");
            }
        }
        for(int i = 0; i < 256; i++) {
            if(temp[i] == '\0') {
                np->code[i] = (flag[depth]%2 == 0)? '1':'0';
                np->code[i+1] = '\0';
                break;
            } else {
                np->code[i] = temp[i];
            }
        }
        if((flag[depth] % 2) == 0) {
            printf("├── ");
        } else {
            printf("└── ");
        }
        
        if(np->symbol == -1) {
            printf("-1\n");
        } else if((char)(np->symbol) == '\n') {
            printf("LF: %s\n", np->code);
        } else {
            printf("%c: %s\n", (char)(np->symbol), np->code);
        }
    }
    flag[depth]++;
    traverse_tree(depth + 1, np->left, np->code);
    traverse_tree(depth + 1, np->right, np->code);
}

// この関数は外部 (main) で使用される (staticがついていない)
Node *encode(const char *filename)
{
    reset_count();
    count_symbols(filename);
    Node *root = build_tree();
    
    if (root == NULL){
	    fprintf(stderr,"A tree has not been constructed.\n");
    }

    return root;
}

// ├── include
// │   ├── qsort.h
// │   │   ├── ear
// │       
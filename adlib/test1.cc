#include "lib.h"

#define CMD "seq"
#define ARGS "10000"

struct Node : public GC {
  Node *left, *right;
  Word val;
  Word dummy[16 - 3];
};

Node *make_tree(int depth, Word &counter) {
  Node *node = new Node();
  if (depth != 0) {
    node->left = make_tree(depth - 1, counter);
    node->val = counter++;
    node->right = make_tree(depth - 1, counter);
  } else {
    node->val = counter++;
  }
  return node;
}

Word sum_tree(Node *node) {
  Word result = 0;
  if (node) {
    result += node->val;
    result += sum_tree(node->left);
    result += sum_tree(node->right);
  }
  return result;
}

static const int iter = 20;
static const int max_depth = 15; // 2^16-1 nodes.

void Main() {
  Str *cmd = new Str(CMD);
  StrArr *args = (new Str(ARGS))->split(' ');
  Word n = 0;
  Str *output;
  for (int j = 0; j < iter; j++) {
    output = ReadProcess(cmd, args);
    n += output->len();
  }
  Check(n == iter * 48894, "read process output as bytes");
  Node *tree;
  static Node *permtree;
  Word counter;
  counter = 0;
  GCVar(permtree, make_tree(max_depth, counter));
  for (int i = 0; i < iter; i++) {
    counter = 0;
    tree = make_tree(max_depth, counter);
  }
  // This may overflow on 32-bit machines, so we use unsigned
  // modulo arithmetic without division.
  Check(sum_tree(tree) * 2 == counter * (counter - 1),
      "stress test memory allocation");
}

#ifndef __LBF_H
#define __LBF_H

#define MAX_CELLS 30000
#define MAX_CODE_SIZE (640 << 10)	// 640k ought to be enough for anyone.

typedef struct bf_jump bf_jump;
struct bf_jump {
	jit_insn *fwd;
	jit_insn *back;
	bf_jump *link;
};

#endif

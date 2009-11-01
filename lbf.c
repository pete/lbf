/*
 * LBF:  A JIT-compiled version of the ever-popular programming language whose
 * real name cannot be given in polite company.
 * 
 * Written from scratch by Pete Elmore (pete.elmore@gmail.com), Halloween 2009,
 * specifically so he could play with lightning some more.
 * 
 * It's public domain.
 */
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include <lightning.h>

#include "lbf.h"

int (*f)();
char cells[MAX_CELLS];
int celli = 0;
jit_insn code_buffer[MAX_CODE_SIZE];
bf_jump *head = 0;

void compile_begin()
{
	f = (int (*)())(jit_set_ip(code_buffer).vptr);

	jit_prolog(0);
}

void compile_end()
{
	if(head) {
		fprintf(stderr, "Syntax error:  Unmatched '['!\n");
		exit(1);
	}

	jit_movi_i(JIT_RET, 0);
	jit_ret();

	jit_flush_code(code_buffer, jit_get_ip().ptr);
}

void compile_inc()
{
	jit_ldi_p(JIT_R0, &celli);
	jit_addi_p(JIT_R0, JIT_R0, cells);
	jit_ldr_uc(JIT_R1, JIT_R0);
	jit_addi_ui(JIT_R1, JIT_R1, 1);
	jit_str_uc(JIT_R0, JIT_R1);
}

void compile_dec()
{
	jit_ldi_p(JIT_R0, &celli);
	jit_addi_p(JIT_R0, JIT_R0, cells);
	jit_ldr_uc(JIT_R1, JIT_R0);
	jit_subi_ui(JIT_R1, JIT_R1, 1);
	jit_str_uc(JIT_R0, JIT_R1);
}

void compile_while()
{
	bf_jump *m;

	m = malloc(sizeof(bf_jump));
	if(!m) {
		fprintf(stderr,
			"Somehow, we ran out of memory compiling your code?\n");
		abort();
	}

	m->link = head;
	head = m;

	jit_ldi_p(JIT_R0, &celli);
	jit_addi_p(JIT_R0, JIT_R0, cells);
	jit_ldr_uc(JIT_R1, JIT_R0);
	m->fwd = jit_beqi_ui(jit_forward(), JIT_R1, 0);
	m->back = jit_get_ip().ptr;
}

void compile_done()
{
	bf_jump *m = head;

	if(!m) {
		fprintf(stderr, "Syntax error:  Unmatched ']'!\n");
		exit(2);
	}

	head = head->link;

	jit_ldi_p(JIT_R0, &celli);
	jit_addi_p(JIT_R0, JIT_R0, cells);
	jit_ldr_uc(JIT_R1, JIT_R0);
	jit_bnei_ui(m->back, JIT_R1, 0);
	jit_patch(m->fwd);
	free(m);
}

// TODO:  Wrap-around when we hit a boundary?
void compile_next()
{
	jit_ldi_p(JIT_R0, &celli);
	jit_addi_i(JIT_R0, JIT_R0, 1);
	jit_sti_p(&celli, JIT_R0);
}

// TODO:  Wrap-around when we hit a boundary?
void compile_prev()
{
	jit_ldi_p(JIT_R0, &celli);
	jit_subi_i(JIT_R0, JIT_R0, 1);
	jit_sti_p(&celli, JIT_R0);
}

void compile_read()
{
	jit_ldi_p(JIT_R0, &celli);
	jit_addi_p(JIT_R0, JIT_R0, cells);
	jit_movi_i(JIT_R1, 1);
	jit_movi_i(JIT_R2, 0);
	jit_str_c(JIT_R0, JIT_R2);

	jit_prepare(3);
		jit_pusharg_i(JIT_R1);
		jit_pusharg_p(JIT_R0);
		jit_pusharg_i(JIT_R2);
	jit_finish(read);
}

void compile_write()
{
	jit_ldi_p(JIT_R0, &celli);
	jit_addi_p(JIT_R0, JIT_R0, cells);
	jit_movi_i(JIT_R1, 1);

	jit_prepare(3);
		jit_pusharg_i(JIT_R1);
		jit_pusharg_p(JIT_R0);
		jit_pusharg_i(JIT_R1);
	jit_finish(write);
}

int main(int argc, char **argv)
{
	int fd;
	char c;

	if(argc != 2) {
		fprintf(stderr, "A JIT-compiled interpeter for BF.\n"
			"Usage:  %s filename.bf\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "Couldn't open %s.\n", argv[1]);
		exit(2);
	}

	compile_begin();
	while(read(fd, &c, 1)) {
		switch (c) {
		case '+':
			compile_inc();
			break;
		case '-':
			compile_dec();
			break;
		case '[':
			compile_while();
			break;
		case ']':
			compile_done();
			break;
		case '>':
			compile_next();
			break;
		case '<':
			compile_prev();
			break;
		case ',':
			compile_read();
			break;
		case '.':
			compile_write();
			break;
		}
	}
	close(fd);
	compile_end();

	return f();
}

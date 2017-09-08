
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int get_tokens(char *buf, char **token1, char **token2) {
	char *p;
	p = strpbrk(buf, " \t");
	if (p) {
		*p++ = 0;
		*token2 = p;
		p = strpbrk(p, "\r\n \t");
		if (p) {
			*p = 0;
		}
		*token1 = buf;
		return 1;
	}
	return 0;
}

static const char *idc_prologue = \
	"#include <idc.idc>\n" \
	"static main(void) {\n" \
	"  auto ea;\n" \
;

static const char *idc_epilogue = \
	"}\n" \
;

int main(int argc, char *argv[]) {
	FILE *fp;
	char buf[1024], cmd[1024];
	char *previous_name, *new_name;

	if (argc == 2) {
		fp = fopen(argv[1], "r");
	} else {
		fp = stdin;
	}
	if (fp) {
		fprintf(stdout, "%s", idc_prologue);
		while (fgets(buf, sizeof(buf), fp)) {
			if (get_tokens(buf, &previous_name, &new_name)) {
				snprintf(cmd, sizeof(cmd), "/bin/sed -i s/%s/%s/g *.cpp *.h", previous_name, new_name);
				fprintf(stderr, "%s\n", cmd);
				system(cmd);
				fprintf(stdout, "  ea = LocByName(\"%s\");\n", previous_name);
				fprintf(stdout, "  if (ea != BADADDR) MakeNameEx(ea, \"%s\", SN_CHECK | SN_NOWARN);\n", new_name);
			}
		}
		fprintf(stdout, "%s", idc_epilogue);
		if (fp != stdin) {
			fclose(fp);
		}
	}
	return 0;
}


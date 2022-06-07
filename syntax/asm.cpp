#include <cstring>
#include <memory>
#include <ctype.h>

static const char * directives[] = {
	"bits","section","segment",".section",".option",".end",".cfi_endproc",".cfi_startproc","org","db","dw",
	"dd","dq","resb","resw","resd","resq",".skip",".byte",".long",".short",".macro",".endm","%include",
	"%incbin",".include",".incbin","%macro","%endmacro","%define","%struc",".size","equ"
};

static const char * keywords[] = {
	// RISC-V instructions
	// TODO: fill

	// Orginal x86 instructions
	"aaa","aad","aam","aas","adc","add","and","call","cbw","clc","cld","cli","cmc","cmp","cmpsb","cmpsw","cwd",
	"daa","das","dec","div","esc","hlt","idiv","imul","in","inc","int","into","iret","jz","jcxz","jmp","lahf",
	"lds","lea","les","lock","lodsb","lodsw","loop","loope","loopne","loopz","loopnz","mov","movsb","movsw","mul",
	"neg","nop","not","or","out","pop","popf","push","pushf","rcl","rcr","rep","repe","repne","repz","repnz","ret",
	"retn","retf","rol","ror","sahf","sal","sar","sbb","scasb","scasw","shl","shr","stc","std","sti","stosb",
	"stosw","sub","test","wait","xchg","xlat","xor","jnz","jnge","jge","jg","jng","jnl","jl","jle","jnle","jb",
	"jbe","jnb","jnbe","ja","jna","jae","jnae"

	// 8013x instructions
	"bound","enter","ins","leave","outs","popa","pusha",

	// 80286 instructions
	"arpl","clts","lar","lgdt","lidt","lldt","lmsw","loadall","lsl","ltr","sgdt","sidt","sldt","smsw","str",
	"verr","verw",

	// 80386 instructions
	"bsf","bsr","bt","btc","btr","bts","cdq","cmpsd","cwde","ibts","insd","iretd","iretf","jecxz","lfs","lgs","lss",
	"lodsd","loopw","loopnew","loopew","loopzw","loopnzw","loopd","loopned","looped","loopzd","loopznd","mov",
	"movsd","movsx","movzx","outsd","popad","popfd","pushad","pushfd","scasd","seta","setae","setb","setbe","setc",
	"sete","setg","setge","setl","setle","setna","setnae","setnb","setnbe","setnc","setne","setng","setnge","setnl",
	"setnle","setno","setnp","setns","setnz","seto","setp","setpe","setpo","sets","setz","shld","shrd","stosd",
	"xbts",

	// 80486 instructions
	"bswap","cmpxchg","invd","invlpg","wbinvd","xadd",

	// Pentium instructions
	"cpuid","cmpxchg8b","rdmsr","rdtsc","wrmsr","rsm",

	// Pentium MMX instructions
	"rdpmc",

	// AMD K6 instructions
	"syscall","sysret",

	// Pentium Pro instructions
	// TODO: add here ...
};

void asm_parser(const char * text, char * style, int length) {
	for(int i = 0; i < length; i++) {
		const char * end_of;
		size_t diff;
		int base;

		if(text[i] == '/' || text[i] == ';' || text[i] == '#') {
			bool is_multi;
			end_of = NULL;
			end_of = strchr(&text[i],'\n');
			is_multi = false;
			if(text[i+1] == '*') { // multi line comment
				end_of = strstr(&text[i],"*/");
				is_multi = true;
			}
			if(end_of == NULL) {
				continue;
			}
			diff = strlen(&text[i])-strlen(end_of);
			if(is_multi) {
				diff += 2;
			}
			memset(&style[i],'C',diff);
			i += diff;
		} else if(text[i] == '\'' || text[i] == '"') {
			// strings require a bit of special parsing
			base = text[i];
			style[i] = 'F';
			i++;
			while(text[i] != base) {
				style[i] = 'F';
				if(text[i] == '\\') {
					style[i+1] = 'F';
					i += 2;
					continue;
				}
				i++;
			}
			style[i] = 'F';
		} else if(text[i] >= '0' && text[i] <= '9') {
			// a number
			while(isalnum(text[i])) {
				style[i] = 'D';
				i++;
			}
		} else {
			// check keywording
			const char * wspace = strpbrk(&text[i]," \t\n;");
			if(wspace != NULL) {
				diff = strlen(&text[i])-strlen(wspace);
				if(!diff) {
					continue;
				}
				auto partText = std::unique_ptr<char[]>(new char[diff+1]);
				strncpy(partText.get(),&text[i],diff);
				partText[diff] = '\0';
				for(size_t j = 0; j < (sizeof(keywords)/sizeof(const char *))-1; j++) {
					if(strcasecmp(keywords[j],partText.get())) {
						continue;
					}
					memset(&style[i],'E',strlen(keywords[j]));
					i += diff;
					break;
				}
				for(size_t j = 0; j < (sizeof(directives)/sizeof(const char *))-1; j++) {
					if(strcasecmp(directives[j],partText.get())) {
						continue;
					}
					memset(&style[i],'D',strlen(directives[j]));
					i += diff;
					break;
				}
				continue;
			}
		}
	}
	return;
}

#include <cstring>
#include <ctype.h>

static const char * keywords[] = {
	"pub","fn","struct","match","if","else","unsafe","return","continue","break",
    "goto","loop","while","trait","using","async","await","let","mut",
	"static","f32","f64","u8","u16","u32","u64","i8","i16","i32","i64",
	"usize","isize","ref","as","try","catch","throw","inline"
    // TODO: I forgot the rest of the rust keywords
};

void rust_parser(const char * text, char * style, int length) {
	for(int i = 0; i < length; i++) {
		const char * end_of;
		size_t diff;
		int base;

		if(text[i] == '#') {
			end_of = strchr(&text[i],'\n');
			if(end_of == NULL) {
				continue;
			}
			diff = strlen(&text[i])-strlen(end_of);
			memset(&style[i],'B',diff);
			i += diff;
		} else if(text[i] == '/') {
			bool is_multi;
			end_of = NULL;
			if(text[i+1] == '/') { // single line comment
				end_of = strchr(&text[i],'\n');
				is_multi = false;
			} else if(text[i+1] == '*') { // multi line comment
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
			// TODO: Parse \" like this
			const size_t start_i = i;
			base = text[i];
			style[i] = 'F';
			i++;
			while(text[i] != base && text[i] != '\n' && i < length-1) {
				style[i] = 'F';
				i++;
			}
			if(text[i] == base) {
				style[i] = 'F';
			} else {
				memset(&style[start_i],'A',i-start_i); // Reset style
			}
		} else if(text[i] >= '0' && text[i] <= '9') {
			// a number
			while(isalnum(text[i])) {
				style[i] = 'D';
				i++;
			}
		} else {
			const char * wspace = strpbrk(&text[i]," \t\n()*&[],\"'.-<>=!#%^;:");
			if(wspace == NULL) {
				continue;
			}
			diff = strlen(&text[i])-strlen(wspace);
			if(!diff) {
				continue;
			}
			memset(&style[i],'A',diff); // Reset style
			for(size_t j = 0; j < (sizeof(keywords)/sizeof(const char *))-1; j++) {
				// check if it matches
				if(strncmp(keywords[j],&text[i],diff) || diff < strlen(keywords[j])) {
					continue;
				}
				memset(&style[i],'E',diff);
				break;
			}
			i += diff;
		}
	}
	return;
}

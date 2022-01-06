#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

// WORDLE OF PLASTIC
// worle
// wordls

#define uint unsigned int

#define WORDLE_SLOT 0
#define WORDLE_IN 1
#define WORDLE_NO 2

int strcon(char* s, char c) {
	uint sl = strlen(s);
	for(uint i = 0; i < sl; i++) if(s[i] == c) return 1;
	return 0;
}

int wsuccess(int8_t* m, uint l) {
	for(uint i = 0; i < l; i++) if(m[i] != WORDLE_SLOT) return 0;
	return 1;
}

uint* gentab(char* s, uint l) {
	uint* r = malloc(sizeof(uint) * 256);
	memset(r, 0, 256);
	for(uint i = 0; i < l; i++) r[s[i]]++;
	return r;
}

int8_t* worle(char* word, char* guess) {
	uint wordlen = strlen(word);
	
	if(strlen(guess) != wordlen) return NULL;

	uint* lctab = gentab(word, wordlen);
	int8_t* ret = malloc(sizeof(int8_t) * wordlen);
	memset(ret, 0xFF, wordlen);
	
	for(uint i = 0; i < wordlen; i++) {
		if(word[i] == guess[i]) {
			ret[i] = WORDLE_SLOT;
			if(lctab[word[i]] != 0) lctab[word[i]]--;
		}
		else {
			if(lctab[guess[i]] != 0) {
				ret[i] = WORDLE_IN;
				lctab[guess[i]]--;
			}
			else ret[i] = WORDLE_NO;
		}
	}

	free(lctab);
	return ret;
}

void clrscr() {
	printf("\x1B\x5B\x48\x1B\x5B\x4A");
}
char* wcol[3] = {
	"\033[32m",
	"\033[33m",
	"\033[31m",
};
void rg(char* word, char* guess, uint l) {
	uint8_t* w = worle(word, guess);
	for(uint i = 0; i < l; i++) printf("%s%c", wcol[w[i]], guess[i]);
	printf("\033[0m\n");
	free(w);
}

void rgw(char* word, char* guess, uint l) {
	uint8_t* w = worle(word, guess);
	char* wc = "GYR";
	for(uint i = 0; i < l; i++) printf("%s%c", wcol[w[i]], wc[w[i]]);
	printf("\033[0m\n");
	free(w);
}

void vword(char* s) {
}

int main(int argc, char** argv) {
	if(argc < 2) printf("argv[1] must contain word\n");
	char* word = argv[1];
	uint l = strlen(word);
	for(uint i = 0; i < l; i++) if(word[i] < 'a' || word[i] > 'z') exit(!!printf("word must be all lowercase letters\n"));
	
	int win = 0;
	fcntl(0, F_SETFL, O_NONBLOCK);
	struct termios t, tsave;
	tcgetattr(0, &t);
	tsave = t;
	t.c_iflag &= ~(ICRNL|IXON);
	t.c_lflag &= ~(ICANON|ECHO|ISIG|IEXTEN);
	tcsetattr(0, TCSANOW, &t);
	setbuf(stdout, NULL);

	char** guesstab = malloc(sizeof(char*));
	uint guesses = 0;

	while(1) {

		guesstab = realloc(guesstab, sizeof(char*) * ++guesses);
		guesstab[guesses - 1] = malloc(sizeof(char) * 51);
		uint guessz = 50, guessl = 0;

		clrscr();
		for(uint s = 0; s < guesses-1; s++) rg(word, guesstab[s], l);
		printf("abcdefghijklmnopqrstuvwxyz");
		printf("\033[%u;%uH", guesses, 1);

		rd:
		int in = 0;
		while(1) {
			unsigned char c;
			do { c = getchar(); } while(c == 255);
			if(c == 'e') {
				break;
			} else {
				if(c == '\x1b') {
					c = getchar();
					if(c == '[') {
						c = getchar();
						switch(c) {
							case 'B':
								if(guessl == 0) break;
								printf("\033[%u;%uH", guesses + 1, 1);
								for(uint i = 0; i < strlen(guesstab[guesses - 1]); i++) putchar(' ');
								guesstab[guesses - 1][--guessl] = 0;
								printf("\033[%u;%uH%s", guesses+1, 1, guesstab[guesses - 1]);
								printf("\033[%u;%uH", guesses, in+1);
								break;
							case 'D':
								if(!in) in = 26;
								in = (in - 1) % 26;
								break;
							case 'C':
								in = (in + 1) % 26;
								break;
						}
						printf("\033[%u;%uH", guesses, in+1);
					}
				} else if(c == ' ') {
					if(guessl == guessz) guesstab[guesses - 1] = realloc(guesstab[guesses - 1], guessz += 50);
					guesstab[guesses - 1][guessl++] = in + 'a';
					guesstab[guesses - 1][guessl] = 0;
					printf("\033[%u;%uH%s", guesses+1, 1, guesstab[guesses - 1]);
					printf("\033[%u;%uH", guesses, in+1);
				} else if(c == 'q') goto end;
			}
		}

	//	printf("WORD == %s\n", word);
	//	printf("GUESS == %s\n", guesstab[guesses - 1]);
		int8_t* res = worle(word, guesstab[guesses - 1]);
		if(!res) {
			printf("\033[%u;%uH", guesses + 2, 1);
			printf("wrong number of characters?\n");
			printf("\033[%u;%uH", guesses + 1, 1);
			for(uint i = 0; i < strlen(guesstab[guesses - 1]); i++) putchar(' ');
			printf("\033[%u;%uH", guesses, 1);
			guessl = 0;
			goto rd;
		}
		else {
			if(wsuccess(res, l)) break;
			free(res);
		}
	}

	win = 1;
end:
	tcsetattr(0, TCSANOW, &tsave);
	clrscr();
	if(win) printf("success! (%s)\n", word);
	for(uint s = 0; s < guesses; s++) {
		if(win) rgw(word, guesstab[s], l);
		free(guesstab[s]);
	}
	free(guesstab);

}

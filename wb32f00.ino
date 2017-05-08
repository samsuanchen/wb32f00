#define PRINTF(format, ...) Serial.printf(format, ## __VA_ARGS__) 

//#include "wb32f10.h"
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv wb32f10.h vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
typedef void (*FuncP) (); // for primative function code pointer
/////////////////////////////////////////////////////////////////////////////////////////////////////////
union P{                  // make the followings use the same address
  int           constant; // value of forth constant 
  int*          variable; // point to forth variable
  int*             value; // point to forth value
  char*           buffer; // point to forth array
  struct Word   **wplist; // point to list of forth words
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Word {     // forth word structure
  struct Word      *link; // point to previous forth word
  int                 id; // id of forth word
  int               flag; // 1 IMMD_WORD, 2 HIDE_WORD
  char             *name; // name of forth word
  FuncP             code; // point to function code
  union P              p; // constant or pointer to variable, value, buffer, or wplist
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Voc {     // forth vocaburary structure
  int               size; // space allocated
  int            last_id; // id of the last defined forth word
  Word             *last; // point to the last defined forth word
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean EOL       (char c      ); // check if c is end of line
boolean baskSpace (char c      ); // check if c is back space
boolean whiteSpace(char c      ); // check if c is white space
boolean tibEmpty  (            ); // check if tib is empty
boolean dsHasItems(int  n      ); // check if data stack has n items
boolean dsHasSpace(int  n      ); // check if data stack has space for n items
boolean dsFull    (            ); // check if data stack full
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void    tibOpen   (char*t,int n); // take n-byte space at t as tib and wait for input
void    tibClose  (            ); // add '\0' at tibEnd
void    tibPop    (            ); // pop last input character from tib
void    tibPush   (char c      ); // collect  input character into tib
void    dsClear   (            ); // reset data stack
void    dsPush    (int  n      ); // push a number onto data stack
int     dsPop     (            ); // pop a number from data stack
int     dsDepth   (            ); // depth of data stack
/////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t hash         (char*str ); // compute non-zero hash of str
char   *makeHash     (char*str ); // get non-zero hash of s in sbuf
void    showCharCodes(char*str ); // show str length and each charCode in str
void    showDataStack(         ); // show data stack info
char    toDigit (int x         ); // oververt integer x into a single digit
char   *toDigits(int x,int base); // convert integer x into digits in given base
void    parseOpen    (char*src ); // reset for parsing
char   *parseToken   (         ); // use white spaces as delimiters to parse a token
char   *hexPrefix    (char*str ); // 0x, 0X, or $ are all acceptable as hexadecimal number prefix
Word   *vocSearch(Voc*voc,char*name); // search name in dict
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ wb32f10.h ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//#include "wb32f10.c"
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv wb32f10.c vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
boolean        EOL(char c){ return c=='\n'||c=='\r'                 ; } // check if c is end of line
boolean  baskSpace(char c){ return c=='\b'                          ; } // check if c is back space
boolean whiteSpace(char c){ return c==' '||c=='\t'||c=='\n'||c=='\r'; } // check if c is white space
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char*tibBegin ; // terminal input buffer begin
char*tibEnd   ; // terminal input buffer end
char*tibLimit ; // terminal input buffer limit
char*tibRemain; // terminal input buffer remain for parsing
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void tibOpen  (char*t,int s){ tibBegin=tibEnd=t,tibLimit=t+s-1; } // init tib and wait for input
void tibClose ()            { *tibEnd='\0'                    ; } // add '\0' at end of input
void tibPop   ()            { --tibEnd                        ; } // pop last input character
void tibPush  (char c)      { *(tibEnd++)=c                   ; } // collect input character
/////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean tibEmpty      (){ return tibEnd==tibBegin; } // check if buffer is empty
boolean tibFull       (){ return tibEnd==tibLimit; } // check if buffer is full
boolean parseAvailable(){ return *tibRemain      ; } // check if tib remain string to parse
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define   SLEN_MAX  252                 // maximum string length
#define   SBUF_LEN 4096                 // string buffer length
char sbuf[SBUF_LEN] ;                   // string buffer to keep all strings
char*sbufRemain=sbuf;                   // remain of string buffer
#define   SBUF_LIMIT (sbuf+SBUF_LEN-1)  // limit of string buffer
/////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t hash(char*str){ // compute non-zero hash of str
  uint8_t sum=0; char*p=str, c=*p++; while(c) sum+=c, c=*p++; if(!sum)sum++; return sum;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char*makeHash(char*str) { // get non-zero hash of s in sbuf
  int n=strlen(str); uint8_t h=hash(str);
  char*p=sbuf+1;                              // point p to the check sum of the first string
  while (p<sbufRemain) {
    if(*p++==h && strcmp(p,str)==0)return p;// return string at p in sbuf
    p+=*(p-2);                                // point p to the check sum of the next string
  }
  if(sbufRemain+n+3 >= SBUF_LIMIT){ // sbuf overflow
    PRINTF("%s\n??? strlen %d, sbuf over %d ???",str,n,sbufRemain+n+3-SBUF_LIMIT); return 0; 
  }
  if(n>SLEN_MAX){ // string length too long
    PRINTF("%s\n??? strlen %d too long > %d ???",str,n,SLEN_MAX); return 0;
  }
  * sbufRemain++=(n+2);                       // point to next check sum
  * sbufRemain++=h;                           // add check sum
  p=sbufRemain, strcpy(p,str), sbufRemain+=n; // add string
  * sbufRemain++=0;                           // add end of string
  return p;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void showCharCodes(char*str){ // show str length and each charCode in str
  int n=strlen(str);
  PRINTF("\n%d byte%s: ",n,n>1?"s":"");    // print str length
  while(*str)PRINTF("%02x ",(char)*str++); // print char codes
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define  TMP_SIZE 256
char tmp[TMP_SIZE]; // for number convertion and token parsing
#define  TMP_LIMIT (tmp+TMP_SIZE)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DS_DEPTH 32
int DS[DS_DEPTH]; // data stack
#define DS_LIMIT (DS+DS_DEPTH-1)        // data stack limit
int*DP=DS-1;      // data stack pointer
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int T;            // top of data stack
int B=10;         // base for number conversion (at least 2, at most 36)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean dsHasItems (int n) { return DP>=DS+n-1       ; } // check if data stack has n items
boolean dsHasSpace (int n) { return DP+n<=DS+DS_DEPTH; } // check if data stack has space for n items
boolean dsFull     ()      { return DP>=DS+DS_DEPTH  ; } // check if data stack full
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void dsClear()      { DP=DS-1       ; } // reset data stack
void dsPush (int n) { T=n, *(++DP)=T; } // push a number onto data stack
int  dsPop  ()      { return *DP--  ; } // pop a number from data stack
int  dsDepth()      { return DP-DS+1; } // depth of data stack
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char toDigit(int x){ // oververt integer x into a single digit
  if(x<10)return '0'+x   ;  // 0-9
  else    return 'a'+x-10;  // a-z
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char*toDigits(int x, int base){ // convert integer x into digits in given base
  char*p=TMP_LIMIT; *(--p)=0;
  if(x==0){ *(--p)='0'; return p; }
  boolean neg; if( neg=(base==10&&x<0) ) x=-x;
  while(x) *(--p)=toDigit(x%base),x/=base; // convert current digit to character
  if(neg) *(--p)='-';
  return p;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void showDataStack(){ // show data stack info
  PRINTF("< dsDepth %d [ ",dsDepth()); // show depth
  if(dsDepth()>0){
    if(dsDepth()>4)PRINTF(".. "); // showing at most top 5 items
    for ( int *i=max(DP-4,DS); i<=DP; i++ ) PRINTF(toDigits(*i,B)),PRINTF(" "); // show data
  }
  PRINTF("] base%d >\n",B); // show base 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void parseOpen(char*src){ // reset for parsing
  tibRemain=tibBegin=src, tibEnd=src+strlen(src);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char*parseToken() { // use white spaces as delimiters to parse a token
  char c=*tibRemain++, *p, *q, *start, *limit; 
  while (c &&  whiteSpace(c)) c=*tibRemain++; start=tibRemain-1; // ignore leading white spaces
  while (c && !whiteSpace(c)) c=*tibRemain++; limit=tibRemain-1; // colect non     white spaces
  if(c){
    p=start, start=q=tmp; while(*p && p<limit)*(q++)=*(p++); *q=0; // make a zero ended string at tmp
  } else tibRemain=limit;
  return start;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char* hexPrefix(char *s) { // 0xff 0XFF $ff $FF $Ff are all acceptable as hexadecimal numbers
  char c;
  if((c=*s++) != '0' && c != '$') return 0;
  if(c=='0' && (c=*s++) != 'x' && c != 'X') return 0;
  return s; // remain string
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define   REVEAL_WORD  0     
#define     IMMD_WORD  1     
#define     HIDE_WORD  2
/////////////////////////////////////////////////////////////////////////////////////////////////////////
Word *vocSearch(Voc *voc, char *name) { // search name in dict
    Word *w=voc->last; while ( w && ( strcmp(w->name,name) || w->flag == HIDE_WORD ) ) w=w->link; return w;
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ wb32f10.c ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//#include voc.c
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv forth vocabulary vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
void _bin(){ B= 2                       ; } const Word W_bin PROGMEM ={NULL         , 1, 0, "bin", _bin, 0};
void _oct(){ B= 8                       ; } const Word W_oct PROGMEM ={(Word*)&W_bin, 1, 0, "oct", _oct, 0};
void _dec(){ B=10                       ; } const Word W_dec PROGMEM ={(Word*)&W_oct, 1, 0, "dec", _dec, 0};
void _hex(){ B=16                       ; } const Word W_hex PROGMEM ={(Word*)&W_dec, 1, 0, "hex", _hex, 0};
void _add(){ dsPush(dsPop()+dsPop())    ; } const Word W_add PROGMEM ={(Word*)&W_hex, 1, 0, "+"  , _add, 0};
void _mul(){ dsPush(dsPop()*dsPop())    ; } const Word W_mul PROGMEM ={(Word*)&W_add, 3, 0, "*"  , _mul, 0};
void _sub(){ T=dsPop(),dsPush(dsPop()-T); } const Word W_sub PROGMEM ={(Word*)&W_mul, 2, 0, "-"  , _sub, 0};
void _div(){ T=dsPop(),dsPush(dsPop()/T); } const Word W_div PROGMEM ={(Word*)&W_sub, 4, 0, "/"  , _div, 0};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
Voc  forth={ 0, 0,(Word*)&W_div };
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ forth vocabulary ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv the main program vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#define  TIB_SIZE 1024
char tib[TIB_SIZE];   // terminal input buffer
int iLine=0, nErr=0 ; // input line index and error count
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void prompt(){
  showDataStack();
  PRINTF("--------------------------------------------------\n");
  PRINTF("input%02d: ",++iLine); // wait for an input string until caridge return
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void eval(char *token){ // 
  char *remain, *p=token, c; Word* w=vocSearch((Voc*)&forth,token);
  if(w){ w->code(); return; }
  int b=B; if(remain=hexPrefix(token)) p=remain, b=16; 
  int n=strtol(p, &remain, b); // conver string at p to integer n on base b (*remain is an illigal digit)
  if(c=*remain) PRINTF("\nerr%02d %s ? illigal '%c' at %d as base%d digit",++nErr,token,c,remain-token,b);
  else dsPush(n);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void interpret(char*src){
  char*token;
  parseOpen(src), showCharCodes(src);
  while(parseAvailable() && (token=parseToken())) eval(token);
  PRINTF("\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200), dsClear();
  PRINTF("==================================================\n");
  PRINTF("Simple Console Parsing Input For Numbers Of Base%d\n",B);
  PRINTF("20170506 derek@wifiboy.org & samsuanchen@gmail.com\n");
  PRINTF("==================================================\n");
  tibOpen(tib,TIB_SIZE),prompt(); // wait for input
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  if (Serial.available()) { char c = Serial.read();     // get input char
    if (baskSpace(c)) {                               // if backspace ------------------------
      if ( !tibEmpty() ) tibPop(),PRINTF("\b \b");          // erase last input char
    } else if ( EOL(c) || tibFull() ) {               // if end of line or buffer full -------
      if ( tibFull() ) PRINTF("\n??? terminal input buffer full ???\n");
      tibClose(), interpret(tib);                           // interpret input string
      tibOpen(tib,TIB_SIZE),prompt();                       // wait for next input
    } else {                                          // else --------------------------------
      tibPush(c), PRINTF("%c",c);                           // collect and echo input char
    }
  }
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ the main program ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

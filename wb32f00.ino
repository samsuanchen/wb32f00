#include "f0_tib.h"       // terminal input buffer handling
#include "f1_str.h"       // unique string handling
#include "f2_dStack.h"    // data stack handling
#include "f3_parse.h"     // parsing number on given base

int iLine=0;             // input line index
void prompt(){ ////////////////////////////////////////////////////////////////////////////////
  showDataStack();
  Serial.println("--------------------------------------------------");
  Serial.printf ("input%02d: ",++iLine);
}
void eval(char*token){ char *remain; //////////////////////////////////////////////////////////
  char *p=token, c; int b=base; if(remain=isHexLeading(token)) p=remain, b=16; 
  int n=strtol(p, &remain, b);
  if(c=*remain) Serial.printf("%s ?? illigal digit %c at %d ",token,c,remain-tibBegin);
  else dsPush(n);
}
void interpret(char*src){ // handle input /////////////////////////////////////////////////////
  parseOpen(src); char*token;
  while(parseAvailable && (token=parseWord())) eval(token);
  parseClose();
}
void setup() { ///////////////////////////////////////////////////////////////////////////////
  Serial.begin(115200);
  Serial.println("==================================================");
  Serial.printf ("A Dumb Console Parsing Input For Numbers Of Base%d\n",base);
  Serial.println("20170427 derek@wifiboy.org & samsuanchen@gmail.com");
  Serial.println("==================================================");
  tibOpen(tib); // wait for input
}
void loop() { ////////////////////////////////////////////////////////////////////////////////
  if (Serial.available()>0) { char c = Serial.read();   // get input char
    if (baskSpace(c)) {                               // if backspace ------------------------
      if (! tibEmpty) tibPop(),Serial.print("\b \b");  //    erase last input char
    } else if (EOL(c) || tibFull) {                   // if end of line or buffer full -------
      if (tibFull) Serial.println("\n??? terminal input buffer full ???");
      tibClose(), interpret(tib), tibOpen(tib);       //    interpret and wait for next input
    } else { // ------------------------------------------------------------------------------
      tibPush(c), Serial.write(c);                     // else collect and echo input char
    }
  }
}

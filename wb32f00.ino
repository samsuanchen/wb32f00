#include    "tsuyu7.h" // img7 周子瑜影像 寬 128 高 160
#include <wifiboy32.h> // wifiboy32_init, fillLCD, WB_BLUE, draw_img
#define BUFLEN 1024
uint16_t pbuf[20480], pwidth, pheight;
inline uint16_t swapByte(uint16_t x) { return (x>>8) | (x<<8); }
void getImg(int w0,int h0,const uint8_t* img,int x,int y,int w,int h){
// 從 w0*h0 的 img 照片位置 x,y 取 w*h 影像 到 pbuf
  uint16_t *t=pbuf; // 目的位址 t 設為 pbuf // 存 w*h pixle 到 ram pbuf (每 pixle 5-bit 紅 6-bit 綠 5-bit 藍)
  uint8_t *s=(uint8_t*)img; s+=(y*w0+x)*2, pwidth=w, pheight=h; // 計算來源位址 s 並 設定 pwidth 與 pheight
  for(int j=0; j<h; j++) memcpy_P(t, s, w*2), t+=w, s+=w0*2; // 從 flash 來源位址 取 w*h 個 pixle 存 pbuf 目的位址
}
void drawBuf(int x,int y){
// 將備存在 ram pbuf 的影像 畫到 屏幕位置 x,y
  setAddrWindow(x,y,x+pwidth-1,y+pheight-1);
  int n=pwidth*pheight; uint16_t* a=pbuf; while(n--)pushColor(swapByte(*a++));
}
char cmdLine[BUFLEN]; // 輸入字串 收集陣列
int n=0, i=0; // cmdLine length // 輸入字串 長度
void drawImg(const uint8_t* img,int w,int h,int x,int y){
// 將 flash img 寬 w 高 h 的影像 畫到 屏幕位置 x,y
  getImg(w,h,img,0,0,w,h), drawBuf(x,y);
}
void setup() { Serial.begin(115200), delay(500);
  wifiboy32_init(); fillLCD(WB_BLUE); // wifiboy32 240x360 屏幕 籃
  drawImg(img7,128,160,56,80); // 周子瑜影像 畫於屏幕 56,80 183,239 之間
  Serial.print("input: ");
}
void loop() { char c, *p;
  if (Serial.available()>0) { c = Serial.read();
    if (c=='\n' || c=='\r') { // line feed or carriage return
      if(n+1>=BUFLEN){
          Serial.println("\n????? terminal input buffer full ?????");
          n=BUFLEN-1;
      }
      cmdLine[n]='\0'; // zero ended string
      Serial.printf("\r\n(line%03d %d byte%s entered)\n",++i,n,n>1?"s":"");
      for(int k=0;k<n;k++)Serial.printf("%02x ",cmdLine[k]);
      Serial.printf("\r\ninput: ");
      n=0;
    } else {
      if (c==8) { // backspace
        if (n>0) n--, Serial.print("\b \b"); // erase last input character
      } else cmdLine[n++]=c, Serial.write(c); // colect and echo input character
    }
  }
}

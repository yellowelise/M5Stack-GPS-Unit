#include <M5Stack.h>
#include <Math.h>


struct point { float x, y; };
struct line  { struct point p1, p2; };

int ccw (struct point p0,
         struct point p1,
         struct point p2)
{
   float dx1,dx2,dy1,dy2;

   dx1 = p1.x-p0.x; 
   dy1 = p1.y-p0.y;
   dx2 = p2.x-p0.x; 
   dy2 = p2.y-p0.y;

   if (dx1*dy2 > dy1*dx2) return +1;
   if (dx1*dy2 < dy1*dx2) return -1;
   if ((dx1*dx2 < 0) || (dy1*dy2 < 0)) return -1; 
   if ((dx1*dx1+dy1*dy1) < (dx2*dx2+dy2*dy2)) return +1;
   return 0;   
}

int intersect(struct line l1, 
              struct line l2)
{
   return ((ccw(l1.p1,l1.p2,l2.p1)*ccw(l1.p1,l1.p2,l2.p2)) <=0) &&
          ((ccw(l2.p1,l2.p2,l1.p1)*ccw(l2.p1,l2.p2,l1.p2)) <=0);
}

void setup() {
  M5.begin();
  // put your setup code here, to run once:
 
}

void loop() {
  // put your main code here, to run repeatedly:
  line l1;
  line l2;
  l1.p1.x = random(1,32000) /100;
  l1.p1.y = random(1,24000) /100;
  l1.p2.x = random(1,32000) /100;
  l1.p2.y = random(1,24000) /100;

  l2.p1.x = random(1,32000) /100;
  l2.p1.y = random(1,24000) /100;
  l2.p2.x = random(1,34000) /100;
  l2.p2.y = random(1,24000) /100;

  
  M5.Lcd.drawLine(round(l1.p1.x),round(l1.p1.y),round(l1.p2.x),round(l1.p2.y),GREEN);

  M5.Lcd.drawLine(round(l2.p1.x),round(l2.p1.y),round(l2.p2.x),round(l2.p2.y),GREEN);
  
  
  M5.Lcd.setCursor(160,120);
  M5.Lcd.print(intersect(l1,l2));
  delay(1000);
  M5.Lcd.clear();
}

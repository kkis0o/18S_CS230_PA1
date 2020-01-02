//void mult_int_32_to_64(unsigned x, unsigned y, unsigned *xyh, unsigned *xyl);

void mult_int_32_to_64(unsigned x, unsigned y, unsigned *xyh, unsigned *xyl)
{
	/* Only use unsigned type here */
	// fill here..

   int posX=1, posY=1, zeroFlag=0;
   if((int)x<0) posX=0;
   if((int)y<0) posY=0;
   if(x==0||y==0) zeroFlag=1;

   if(zeroFlag==1) return;
   if(posX!=1) x=~x+1;
   if(posY!=1) y=~y+1;

   for(int i=0;i<32;i++)
   {
       unsigned digitY=(y<<31-i)>>31;
       if(digitY==0) continue;
       unsigned part=x&0xFFFFFFFF;
       //printf("part: %08X\n", part);
       unsigned low=part<<i;
       unsigned high=0;
       if(i!=0) high=part>>32-i;
       unsigned lowsum=low+*xyl;
       //printf("i: %d, digit: %d, low: %08X, high: %08X\n",i, digitY, low, high);
       *xyh+=high;
       if(lowsum<low||lowsum<*xyl)
       {
           *xyh+=1;
           *xyl=lowsum;
       }else{
           *xyl+=low;
       }
   }

   if((posY==1&&posX!=1)||(posX==1&&posY!=1))
   {
       *xyh=~(*xyh);
       *xyl=~(*xyl)+1;
   }


}

void float_add(unsigned x, unsigned y, unsigned *result)
{
	/* Only use unsigned type here */
	// fill here..
    *result=0;
    int zeroX=0, zeroY=0;
    
    unsigned signX=((x&0x80000000)==0x80000000)?1:0;
    unsigned signY=((y&0x80000000)==0x80000000)?1:0;
    unsigned expX=(x&0x7F800000)>>23;
    unsigned expY=(y&0x7F800000)>>23;
    //printf("signX: %d, signY: %d\n", signX, signY);
    //printf("expX: %d, expY: %d\n", expX, expY);
    unsigned mtsX=(x&0x007FFFFF)|0x00800000;
    unsigned mtsY=(y&0x007FFFFF)|0x00800000;
    //printf("mtsX: %08X, mtsY: %08X\n", mtsX, mtsY);
    if(expX==0&&mtsX==0x00800000) zeroX=1;
    if(expY==0&&mtsY==0x00800000) zeroY=1;

    if(zeroX==1){
        *result=y;
        return;
    }
    if(zeroY==1){
        *result=x;
        return;
    }

    if(expX<expY){
        mtsX=mtsX>>(expY-expX);
        expX=expY;
    }
    else{
        mtsY=mtsY>>(expX-expY);
        expY=expX;
    }
    //printf("updated exp: %d %d\n", expX, expY);
    unsigned resultExp=expX;
    unsigned resultSign=0;
    if(signX==1&&signY==1) resultSign=1;
    else if(signX!=signY){
        if(mtsX>mtsY) resultSign=signX;
        else resultSign=signY;
    }
    //printf("resExp: %d, resSign: %d\n", resultExp, resultSign);
    if(signX==1) mtsX=~mtsX+1;
    if(signY==1) mtsY=~mtsY+1;

    unsigned addMts=mtsX+mtsY;
    //printf("mtsX: %08X, mtsY: %08X, addMts: %08X\n", mtsX, mtsY, addMts);

    if(addMts==0) return;
    if(resultSign==1) addMts=~addMts+1;
    
    if((addMts&0x01000000)==0x01000000){
        addMts=addMts>>1;
        resultExp+=1;
    }

    while((addMts&0x00800000)!=0x00800000){
        addMts=addMts<<1;
        resultExp-=1;
    } 

    unsigned resultFrac=addMts&0x007FFFFF;
    //printf("resFrac: %08X\n", resultFrac);
    *result=(*result)|(resultSign<<31);
    *result=(*result)|(resultExp<<23);
    *result=(*result)|resultFrac;
    
}

void float_mult(unsigned x, unsigned y, unsigned *result)
{
	/* Only use unsigned type here */
	// fill here..
   *result=0;
   unsigned signX=(x&0x80000000)>>31;
   unsigned signY=(y&0x80000000)>>31;
   unsigned signR=(signX==signY)?0:1;

   unsigned expX=(x&0x7F800000)>>23;
   unsigned expY=(y&0x7F800000)>>23;
   //printf("expX: %d, expY: %d\n", expX, expY);
   unsigned mtsX=(x&0x007FFFFF)|0x00800000;
   unsigned mtsY=(y&0x007FFFFF)|0x00800000;
   //printf("mtsX: %08X, mtsY: %08X\n", mtsX, mtsY);
   if(expX==0&&mtsX==0x00800000) return;
   if(expY==0&&mtsY==0x00800000) return;

   //if(signX==1) mtsX=~mtsX+1;
   //if(signY==1) mtsY=~mtsY+1;
   
   unsigned E=expX+expY-127;
   unsigned xyl=0, xyh=0;
   mult_int_32_to_64(mtsX, mtsY, (unsigned *)&xyh, (unsigned *)&xyl);
//   printf("xyh: %08X, xyl: %08X\n", xyh, xyl);
   unsigned concat;
   if((xyh&0x00008000)==0x00008000){
       E+=1;
       unsigned fromH=xyh&0x0000FFFF; // 16
       //unsigned fromL=xyl&0xFF000000; // 8
       if((xyl&0x00800000)==0x00800000) xyl+=0x00800000;
       unsigned fromL=xyl&0xFF000000;
       concat=(fromH<<8)|(fromL>>24);
   }
   else{
       unsigned fromH=xyh&0x00007FFF; // 15
       if((xyl&0x00400000)==0x00400000) xyl+=0x00400000;
       unsigned fromL=xyl&0xFF800000; // 9
       concat=(fromH<<9)|(fromL>>23);
   }
   concat=concat&0x007FFFFF; // 23
   //printf("signR: %d, E: %d, concat: %08X\n", signR, E, concat);
   *result=(*result)|(signR<<31);
   *result=(*result)|(E<<23);
   *result=(*result)|concat;
   //printf("result: %08X\n", *result);

}

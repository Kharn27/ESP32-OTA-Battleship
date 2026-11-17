/* J3 export helper — RFC1950/1951 + PNG RGBA 8-bit, no external JS dependencies.
 * Pixel work mirrors export_jalon3.py. This file is for transport/QA of
 * candidates by the GitHub+Adobe connectors, not for firmware use. */
function inflateZlib(z,capacity){
 let p=16, o=0, out=new Uint8Array(capacity);
 function bits(n){let v=0;for(let i=0;i<n;i++){if(p>=z.length*8)throw Error("DEFLATE EOF");v|=((z[p>>3]>>(p&7))&1)<<i;p++;}return v}
 function tree(lens){let max=Math.max(...lens),counts=new Uint16Array(max+1),next=new Uint16Array(max+1),maps=Array.from({length:max+1},()=>new Map);for(let l of lens)if(l)counts[l]++;let code=0;for(let l=1;l<=max;l++){code=(code+counts[l-1])*2;next[l]=code}for(let s=0;s<lens.length;s++){let l=lens[s];if(l)maps[l].set(next[l]++,s)}return{maps,max}}
 function sym(t){let code=0;for(let len=1;len<=t.max;len++){code=(code<<1)|bits(1);let s=t.maps[len].get(code);if(s!==undefined)return s}throw Error("Invalid Huffman code at bit "+p)}
 const lbase=[3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258],
 lextra=[0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0],
 dbase=[1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577],
 dextra=[0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13];
 let final=false;
 while(!final){
  final=bits(1)===1;let typ=bits(2);
  if(typ===0){p=(p+7)&~7;let n=bits(16),nn=bits(16);if(((n^nn)&65535)!==65535)throw Error("bad block length");for(let i=0;i<n;i++)out[o++]=bits(8);continue}
  let lit,dist;
  if(typ===1){lit=tree(Array.from({length:288},(_,i)=>i<144?8:i<256?9:i<280?7:8));dist=tree(Array(32).fill(5))}
  else if(typ===2){
   let nl=bits(5)+257,nd=bits(5)+1,nc=bits(4)+4,order=[16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15],cl=Array(19).fill(0);
   for(let i=0;i<nc;i++)cl[order[i]]=bits(3);
   let t=tree(cl),r=[];while(r.length<nl+nd){let s=sym(t);if(s<16)r.push(s);else{let count=s===16?3+bits(2):s===17?3+bits(3):11+bits(7),v=s===16?r[r.length-1]:0;for(let i=0;i<count;i++)r.push(v)}}
   lit=tree(r.slice(0,nl));dist=tree(r.slice(nl,nl+nd));
  }else throw Error("reserved block type");
  while(true){let s=sym(lit);if(s<256){out[o++]=s;continue}if(s===256)break;let i=s-257;if(i>=lbase.length)throw Error("bad length");let count=lbase[i]+bits(lextra[i]),ds=sym(dist);if(ds>=dbase.length)throw Error("bad distance");let back=dbase[ds]+bits(dextra[ds]);if(back>o)throw Error("backref before buffer");for(let j=0;j<count;j++)out[o]=out[o++-back]}
 }
 if(o!==capacity)throw Error("decompressed "+o+", expected "+capacity);return out
}
function decodePng(buffer){
 const b=buffer instanceof Uint8Array?buffer:Uint8Array.fromBase64(buffer),dv=new DataView(b.buffer,b.byteOffset,b.byteLength);
 if(b[0]!==137||b[1]!==80||b[2]!==78||b[3]!==71)throw Error("Not PNG");
 let off=8,w=0,h=0,ids=[];
 while(off<b.length){let n=dv.getUint32(off),tag=String.fromCharCode(...b.slice(off+4,off+8)),dat=b.slice(off+8,off+8+n);off+=12+n;
  if(tag==="IHDR"){const d=new DataView(dat.buffer,dat.byteOffset,dat.byteLength);w=d.getUint32(0);h=d.getUint32(4);if(dat[8]!==8||dat[9]!==6||dat[10]!==0)throw Error("Not PNG RGBA8");}
  if(tag==="IDAT")ids.push(dat);if(tag==="IEND")break;
 }
 const zip=new Uint8Array(ids.reduce((n,x)=>n+x.length,0));let z=0;for(const c of ids){zip.set(c,z);z+=c.length}
 const stride=w*4,raw=inflateZlib(zip,(stride+1)*h),pixels=new Uint8Array(stride*h);
 for(let y=0;y<h;y++){let filter=raw[y*(stride+1)],row=y*stride;for(let x=0;x<stride;x++){
  let cur=raw[y*(stride+1)+1+x],a=x>=4?pixels[row+x-4]:0,up=y?pixels[row-stride+x]:0,ul=(y&&x>=4)?pixels[row-stride+x-4]:0,pred=0;
  if(filter===1)pred=a;else if(filter===2)pred=up;else if(filter===3)pred=(a+up)>>1;else if(filter===4){let pa=Math.abs(up-ul),pb=Math.abs(a-ul),pc=Math.abs(a+up-2*ul);pred=pa<=pb&&pa<=pc?a:pb<=pc?up:ul}
  else if(filter!==0)throw Error("PNG filter "+filter);
  pixels[row+x]=(cur+pred)&255
 }}
 return{w,h,pixels}
}
const crcTable=(()=>{let t=new Uint32Array(256);for(let n=0;n<256;n++){let c=n;for(let k=0;k<8;k++)c=c&1?0xedb88320^(c>>>1):c>>>1;t[n]=c>>>0}return t})();
function crc32(buf){let c=0xffffffff;for(let v of buf)c=crcTable[(c^v)&255]^(c>>>8);return(c^0xffffffff)>>>0}
function addU32(b,i,n){b[i]=n>>>24;b[i+1]=(n>>>16)&255;b[i+2]=(n>>>8)&255;b[i+3]=n&255}
function pngRGBA(w,h,pixels){
 if(pixels.length!==w*h*4)throw Error("PNG pixels count");
 let stride=w*4,raw=new Uint8Array((stride+1)*h);for(let y=0;y<h;y++)raw.set(pixels.subarray(y*stride,(y+1)*stride),y*(stride+1)+1);
 let blocks=Math.ceil(raw.length/65535),zip=new Uint8Array(2+raw.length+blocks*5+4),z=0;zip[z++]=0x78;zip[z++]=0x01;
 for(let off=0;off<raw.length;off+=65535){let n=Math.min(65535,raw.length-off);zip[z++]=off+n===raw.length?1:0;zip[z++]=n&255;zip[z++]=n>>>8;zip[z++]=(~n)&255;zip[z++]=((~n)>>>8)&255;zip.set(raw.subarray(off,off+n),z);z+=n}
 let a=1,bb=0;for(let q=0;q<raw.length;q++){a=(a+raw[q])%65521;bb=(bb+a)%65521}addU32(zip,z,(bb<<16)|a);
 function chunk(tag,data){let name=Uint8Array.from(tag.split("").map(c=>c.charCodeAt(0))),result=new Uint8Array(12+data.length);addU32(result,0,data.length);result.set(name,4);result.set(data,8);let check=new Uint8Array(name.length+data.length);check.set(name);check.set(data,4);addU32(result,8+data.length,crc32(check));return result}
 let hdr=new Uint8Array(13);addU32(hdr,0,w);addU32(hdr,4,h);hdr[8]=8;hdr[9]=6;let chunks=[Uint8Array.from([137,80,78,71,13,10,26,10]),chunk("IHDR",hdr),chunk("IDAT",zip),chunk("IEND",new Uint8Array())],out=new Uint8Array(chunks.reduce((n,c)=>n+c.length,0)),p=0;for(const c of chunks){out.set(c,p);p+=c.length}return out
}
function makeMaster(crop,side,pad){if(crop.w!==side||pad+crop.h>side)throw Error("inconsistent crop "+crop.w+"x"+crop.h+" side "+side+" pad "+pad);let out=new Uint8Array(side*side*4);for(let y=0;y<crop.h;y++)out.set(crop.pixels.subarray(y*crop.w*4,(y+1)*crop.w*4),(y+pad)*side*4);let hidden=0;for(let k=0;k<out.length;k+=4)if(!out[k+3]){if(out[k]||out[k+1]||out[k+2])hidden++;out[k]=out[k+1]=out[k+2]=0}return{pixels:out,hidden}}
function maskRect(data,side,rects){let removed=0;for(const [x0,y0,x1,y1] of rects){if(x0<0||y0<0||x1>side||y1>side)throw Error("mask out of bounds");for(let y=y0;y<y1;y++)for(let x=x0;x<x1;x++){let p=(y*side+x)*4;if(data[p+3])removed++;data[p]=data[p+1]=data[p+2]=data[p+3]=0}}return removed}
function boxAlpha(m,side,target){
 let dst=new Uint8Array(target*target*4),denom=side*side;
 for(let y=0;y<target;y++){let y0=Math.floor(y*side/target),y1=Math.floor(((y+1)*side+target-1)/target);
 for(let x=0;x<target;x++){let x0=Math.floor(x*side/target),x1=Math.floor(((x+1)*side+target-1)/target),sa=0,sr=0,sg=0,sb=0;
  for(let yy=y0;yy<y1;yy++){let wy=Math.max(0,Math.min((yy+1)*target,(y+1)*side)-Math.max(yy*target,y*side));
  for(let xx=x0;xx<x1;xx++){let wx=Math.max(0,Math.min((xx+1)*target,(x+1)*side)-Math.max(xx*target,x*side)),p=(yy*side+xx)*4,v=m[p+3]*wx*wy;if(!v)continue;sa+=v;sr+=m[p]*v;sg+=m[p+1]*v;sb+=m[p+2]*v}}
  let a=Math.floor((sa+Math.floor(denom/2))/denom);if(a&&sa){let p=(y*target+x)*4;dst[p]=Math.floor((sr+Math.floor(sa/2))/sa);dst[p+1]=Math.floor((sg+Math.floor(sa/2))/sa);dst[p+2]=Math.floor((sb+Math.floor(sa/2))/sa);dst[p+3]=a}
 }}
 return dst
}
function nearest4(data,side){let dest=new Uint8Array(side*4*side*4*4);for(let y=0;y<side*4;y++)for(let x=0;x<side*4;x++){let s=((y>>2)*side+(x>>2))*4,d=(y*(side*4)+x)*4;dest[d]=data[s];dest[d+1]=data[s+1];dest[d+2]=data[s+2];dest[d+3]=data[s+3]}return dest}
function alphaStats(data,side){let zero=0,partial=0,opaque=0,x0=side,y0=side,x1=-1,y1=-1;for(let y=0;y<side;y++)for(let x=0;x<side;x++){let a=data[(y*side+x)*4+3];if(a===0)zero++;else{if(a===255)opaque++;else partial++;x0=Math.min(x0,x);y0=Math.min(y0,y);x1=Math.max(x1,x);y1=Math.max(y1,y)}}return{alpha_0:zero,alpha_partial:partial,alpha_255:opaque,bbox_nonzero:x1<0?null:[x0,y0,x1,y1]}}
function components(data,side,minAlpha=6){let seen=new Uint8Array(side*side),groups=[];for(let i=0;i<side*side;i++){if(seen[i]||data[i*4+3]<minAlpha)continue;let queue=[i],pos=0,x0=side,y0=side,x1=0,y1=0;seen[i]=1;while(pos<queue.length){let p=queue[pos++],x=p%side,y=Math.floor(p/side);x0=Math.min(x0,x);y0=Math.min(y0,y);x1=Math.max(x1,x);y1=Math.max(y1,y);for(const q of [x? p-1:-1,x<side-1?p+1:-1,y?p-side:-1,y<side-1?p+side:-1])if(q>=0&&!seen[q]&&data[q*4+3]>=minAlpha){seen[q]=1;queue.push(q)}}groups.push({pixels:queue.length,bbox:[x0,y0,x1,y1]})}groups.sort((a,b)=>b.pixels-a.pixels);return groups}
function blank(w,h,color){let out=new Uint8Array(w*h*4),[r,g,b]=color;for(let i=0;i<out.length;i+=4){out[i]=r;out[i+1]=g;out[i+2]=b;out[i+3]=255}return out}
function rectFill(dst,w,x,y,ww,hh,c){for(let yy=y;yy<y+hh;yy++)for(let xx=x;xx<x+ww;xx++){let p=(yy*w+xx)*4;dst[p]=c[0];dst[p+1]=c[1];dst[p+2]=c[2];dst[p+3]=255}}
const glyphs={"0":["111","101","101","101","111"],"1":["010","110","010","010","111"],"2":["111","001","111","100","111"],"3":["111","001","111","001","111"],"4":["101","101","111","001","001"],"5":["111","100","111","001","111"],"6":["111","100","111","101","111"],"7":["111","001","010","010","010"],"8":["111","101","111","101","111"],"9":["111","101","111","001","111"],"A":["010","101","111","101","101"],"B":["110","101","110","101","110"],"C":["011","100","100","100","011"],"D":["110","101","101","101","110"],"E":["111","100","110","100","111"],"L":["100","100","100","100","111"],"-":["000","000","111","000","000"]," ":["000","000","000","000","000"]};
function label(dst,w,text,x,y,c){for(const ch of text){let g=glyphs[ch]||glyphs[" "];for(let yy=0;yy<5;yy++)for(let xx=0;xx<3;xx++)if(g[yy][xx]==="1"){let p=((y+yy)*w+x+xx)*4;dst[p]=c[0];dst[p+1]=c[1];dst[p+2]=c[2];dst[p+3]=255}x+=4}}
function stamp(dst,w,sprite,side,x,y){for(let yy=0;yy<side;yy++)for(let xx=0;xx<side;xx++){let s=(yy*side+xx)*4,d=((y+yy)*w+(x+xx))*4,a=sprite[s+3];if(!a)continue;for(let c=0;c<3;c++)dst[d+c]=Math.floor((sprite[s+c]*a+dst[d+c]*(255-a)+127)/255)}}
function proofPage(order,variants,offset,bg){let name=bg==="sombre",c=name?[17,31,46]:[225,236,243],ink=name?[221,234,244]:[34,53,68],dst=blank(320,240,c);for(let j=0;j<4;j++){let id=order[offset+j],x=8+j*80;label(dst,320,id,x,5,ink);stamp(dst,320,variants[id][48],48,x+13,24);stamp(dst,320,variants[id][64],64,x+5,123)}label(dst,320,"48",2,85,ink);label(dst,320,"64",2,211,ink);return pngRGBA(320,240,dst)}
function proofContact(order,variants){let dst=blank(1120,182,[34,48,65]);for(let i=0;i<order.length;i++){let id=order[i],x=i*140;label(dst,1120,id,x+2,4,[250,242,208]);rectFill(dst,1120,x+2,20,62,58,[13,28,47]);rectFill(dst,1120,x+73,20,62,58,[226,237,245]);stamp(dst,1120,variants[id][48],48,x+9,25);stamp(dst,1120,variants[id][48],48,x+80,25);rectFill(dst,1120,x+1,99,68,68,[13,28,47]);rectFill(dst,1120,x+71,99,68,68,[226,237,245]);stamp(dst,1120,variants[id][64],64,x+3,101);stamp(dst,1120,variants[id][64],64,x+73,101)}return pngRGBA(1120,182,dst)}
function sha256(bytes){const K=[0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2];let H=[0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19];let n=bytes.length,total=(n+9+63)&~63,buf=new Uint8Array(total);buf.set(bytes);buf[n]=128;let bd=new DataView(buf.buffer);bd.setUint32(total-8,Math.floor((n*8)/4294967296));bd.setUint32(total-4,(n*8)>>>0);let W=new Uint32Array(64);function rr(a,n){return(a>>>n)|(a<<(32-n))}for(let off=0;off<total;off+=64){for(let i=0;i<16;i++)W[i]=bd.getUint32(off+i*4);for(let i=16;i<64;i++){let a=W[i-15],b=W[i-2],s0=rr(a,7)^rr(a,18)^(a>>>3),s1=rr(b,17)^rr(b,19)^(b>>>10);W[i]=(W[i-16]+s0+W[i-7]+s1)>>>0}let[a,b,c,d,e,f,g,h]=H;for(let i=0;i<64;i++){let S1=rr(e,6)^rr(e,11)^rr(e,25),ch=(e&f)^((~e)&g),t1=(h+S1+ch+K[i]+W[i])>>>0,S0=rr(a,2)^rr(a,13)^rr(a,22),maj=(a&b)^(a&c)^(b&c),t2=(S0+maj)>>>0;h=g;g=f;f=e;e=(d+t1)>>>0;d=c;c=b;b=a;a=(t1+t2)>>>0}let x=[a,b,c,d,e,f,g,h];H=H.map((v,i)=>(v+x[i])>>>0)}return H.map(x=>x.toString(16).padStart(8,"0")).join("")}
return {decodePng,pngRGBA,makeMaster,maskRect,boxAlpha,nearest4,alphaStats,components,proofPage,proofContact,sha256};

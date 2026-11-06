#!/usr/bin/env node
/* CYD WARSHIP — Re-export edited pixel rects from the production SVG.
   Uses no external libraries. 24-bit opaque PNG, exact NEAREST x4.
   The SVG, not generated random data, is the editable source of truth. */
const fs=require('fs'),path=require('path');
const dir=__dirname, source=fs.readFileSync(path.join(dir,'ocean_bg_source.svg'),'utf8');
const rgb=new Uint8Array(320*240*3);
const pattern=/<rect x="(-?\d+)" y="(-?\d+)" width="(\d+)" height="(\d+)" fill="(#[0-9a-fA-F]{6})"\/>/g;
let match,count=0;
while((match=pattern.exec(source))!==null){
 const x=+match[1],y=+match[2],w=+match[3],h=+match[4],c=match[5];
 const r=parseInt(c.slice(1,3),16),g=parseInt(c.slice(3,5),16),b=parseInt(c.slice(5,7),16);
 for(let yy=Math.max(0,y);yy<Math.min(240,y+h);yy++)for(let xx=Math.max(0,x);xx<Math.min(320,x+w);xx++){
 const p=(yy*320+xx)*3;rgb[p]=r;rgb[p+1]=g;rgb[p+2]=b;
 }count++;
}
if(count<1000)throw Error('SVG rect count suspicious: '+count);
function oceanCrop(rgb,x0,y0,w,h,stride=320){let out=new Uint8Array(w*h*3);for(let y=0;y<h;y++)for(let x=0;x<w;x++){let a=((y+y0)*stride+x+x0)*3,b=(y*w+x)*3;out[b]=rgb[a];out[b+1]=rgb[a+1];out[b+2]=rgb[a+2];}return out;}
function oceanZoom(rgb,w,h,f=4){let out=new Uint8Array(w*f*h*f*3),nw=w*f;for(let y=0;y<h*f;y++)for(let x=0;x<nw;x++){let a=(Math.floor(y/f)*w+Math.floor(x/f))*3,b=(y*nw+x)*3;out[b]=rgb[a];out[b+1]=rgb[a+1];out[b+2]=rgb[a+2];}return out;}
function oceanPng(rgb,w,h){
 let raw=new Uint8Array(h*(w*3+1));for(let y=0;y<h;y++)raw.set(rgb.subarray(y*w*3,(y+1)*w*3),y*(w*3+1)+1);
 const lenBase=[3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258];
 const lenExtra=[0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0];
 const distBase=[1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577];
 const distExtra=[0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13];
 let def=[120,1], bits=0,nb=0;
 const wb=(x,n)=>{bits|=(x<<nb);nb+=n;while(nb>=8){def.push(bits&255);bits>>>=8;nb-=8;}};
 const rev=(v,n)=>{let r=0;for(let i=0;i<n;i++){r=(r<<1)|(v&1);v>>>=1;}return r;};
 const sym=v=>{let c,n;if(v<=143){c=48+v;n=8;}else if(v<=255){c=400+v-144;n=9;}else if(v<=279){c=v-256;n=7;}else{c=192+v-280;n=8;}wb(rev(c,n),n);};
 wb(3,3);
 const last=new Int32Array(1<<16);last.fill(-1);
 const hash=i=>((raw[i]*257+raw[i+1]*17+raw[i+2]*13)&65535);
 let p=0;
 while(p<raw.length){let ml=0,md=0;
  if(p+2<raw.length){let h=hash(p),q=last[h];if(q>=0&&p-q<=32768){let limit=Math.min(258,raw.length-p);while(ml<limit&&raw[p+ml]===raw[q+ml])ml++;if(ml<3)ml=0;else md=p-q;}}
  if(ml>=3){let j=0;while(j<28&&lenBase[j+1]<=ml)j++;sym(257+j);wb(ml-lenBase[j],lenExtra[j]);j=0;while(j<29&&distBase[j+1]<=md)j++;wb(rev(j,5),5);wb(md-distBase[j],distExtra[j]);
   for(let k=0;k<ml;k++)if(p+k+2<raw.length)last[hash(p+k)]=p+k;p+=ml;
  }else{sym(raw[p]);if(p+2<raw.length)last[hash(p)]=p;p++;}
 }
 sym(256);if(nb)def.push(bits&255);
 let a=1,b=0;for(let v of raw){a=(a+v)%65521;b=(b+a)%65521;}
 def.push((b>>>8)&255,b&255,(a>>>8)&255,a&255);
 let crcTable=new Uint32Array(256);for(let n=0;n<256;n++){let c=n;for(let j=0;j<8;j++)c=c&1?(0xedb88320^(c>>>1)):(c>>>1);crcTable[n]=c>>>0;}
 const BE=n=>[(n>>>24)&255,(n>>>16)&255,(n>>>8)&255,n&255];
 const ck=(type,data)=>{let t=[...type].map(c=>c.charCodeAt(0)),c=0xffffffff;for(let v of [...t,...data])c=crcTable[(c^v)&255]^(c>>>8);return [...BE(data.length),...t,...data,...BE((c^0xffffffff)>>>0)];};
 return new Uint8Array([137,80,78,71,13,10,26,10,...ck('IHDR',[...BE(w),...BE(h),8,2,0,0,0]),...ck('IDAT',def),...ck('IEND',[])]);
}
fs.writeFileSync(path.join(dir,'ocean_bg_full_320x240.png'),oceanPng(rgb,320,240));
fs.writeFileSync(path.join(dir,'ocean_bg_full_x4.png'),oceanPng(oceanZoom(rgb,320,240),1280,960));
fs.writeFileSync(path.join(dir,'ocean_bg_battle_180x180.png'),oceanPng(oceanCrop(rgb,21,53,180,180),180,180));
console.log('PASS: rendered '+count+' SVG pixel rectangles; 320x240, x4 and 180x180.');

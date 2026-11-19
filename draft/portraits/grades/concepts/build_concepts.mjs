import fs from 'node:fs';
import path from 'node:path';
const out=process.argv.find((v,i)=>i>1 && !v.startsWith('--'))||'draft/portraits/grades/concepts';
const check=process.argv.includes('--check');
if(!check) fs.mkdirSync(out,{recursive:true});
function emit(file,contents){const target=path.join(out,file);if(check){if(!fs.existsSync(target)||fs.readFileSync(target,'utf8')!==contents)throw Error('NON REPRODUCTIBLE: '+file);}else fs.writeFileSync(target,contents);}
const C={ink:'#0c1726',black:'#101727',deep:'#162437',deep2:'#223449',navy:'#263b51',teal:'#316173',soft:'#4f8290',pale:'#b2c8bd',gold:'#e8b35d',light:'#ffd58b',amber:'#bf7944',copper:'#985634',rust:'#753a39',red:'#a5483e',scarlet:'#d86748',cream:'#f3dbad',ivory:'#ead7bd',white:'#fff0d1'};
const r=(x,y,w,h,c)=>`<rect x="${x}" y="${y}" width="${w}" height="${h}" fill="${c}"/>`;
const p=(d,c)=>`<path d="${d}" fill="${c}"/>`;
const ln=(d,c,w=1)=>`<path d="${d}" fill="none" stroke="${c}" stroke-width="${w}" stroke-linecap="square" stroke-linejoin="miter"/>`;
function seaUniform(grade,gender){
let a=[];
// common structural silhouette, identical framing in all grades
let base=grade==='mousse'?C.deep2:grade==='capitaine'?C.deep:C.black;
a.push(p('M 3 96 L 5 83 11 76 20 70 29 67 67 67 77 70 86 76 91 84 93 96 Z',C.ink));
a.push(p('M 6 96 L 8 84 14 76 22 72 32 70 64 70 75 72 83 78 89 85 90 96 Z',base));
a.push(p('M 7 93 L 12 82 20 77 26 77 24 96 8 96 Z',grade==='mousse'?'#3b6470':'#233a51'));
a.push(p('M 74 76 L 82 81 88 92 88 96 72 96 70 77 Z',grade==='mousse'?'#253f53':'#17283e'));
a.push(p('M 14 81 L 22 75 27 76 21 86 14 90 Z','#557582'));
a.push(p('M 75 78 L 83 85 86 89 82 83 Z','#425d65'));
a.push(p('M 33 66 L 33 84 43 94 52 94 62 83 62 66 Z','#101b2d'));
if(grade==='mousse'){
 a.push(p('M 23 72 L 32 69 45 85 44 96 31 96 25 86 Z','#496e7b'));
 a.push(p('M 64 69 L 74 73 71 89 62 96 51 96 51 85 Z','#34576b'));
 a.push(p('M 37 75 L 43 81 49 86 56 81 60 75 57 71 49 79 41 70 Z','#e0bd8d'));
 a.push(p('M 38 79 L 48 88 57 78 60 82 49 94 43 90 Z','#ce8d65'));
 a.push(ln('M 17 91 L 24 78 M 77 77 L 83 92','#76919a',2));
 a.push(r(15,87,5,2,'#d6aa69'));a.push(r(77,86,5,2,'#d6aa69'));
 a.push(ln('M 28 73 L 39 86','#e4be7c',2));
 a.push(p('M 31 76 L 39 83 36 88 29 80 Z','#b46f4f'));
}else if(grade==='capitaine'){
 a.push(p('M 23 73 L 36 67 46 88 43 96 27 96 Z','#375a68'));
 a.push(p('M 73 73 L 62 67 51 88 55 96 72 96 Z','#304d5e'));
 a.push(p('M 38 71 L 48 85 56 71 60 72 50 92 46 92 34 73 Z','#efe1bf'));
 a.push(ln('M 28 78 L 36 95 M 68 78 L 59 95',C.gold,2));
 a.push(p('M 10 77 L 27 67 34 70 29 78 11 85 Z','#132337'));
 a.push(p('M 69 69 L 77 68 86 78 69 77 63 71 Z','#152438'));
 a.push(ln('M 13 76 L 25 70 L 30 71 M 69 70 L 77 71 L 82 76',C.gold,2));
 a.push(r(45,88,4,4,C.gold));a.push(r(54,93,3,3,C.gold));a.push(r(40,93,3,3,C.gold));
 a.push(r(44,89,1,1,C.white));
 a.push(p('M 74 78 L 81 80 82 84 72 81 Z','#b28c4d'));
}else if(grade==='amiral'){
 a.push(p('M 25 71 L 35 68 46 87 43 96 24 96 Z','#2f465b'));
 a.push(p('M 71 71 L 62 68 51 87 54 96 73 96 Z','#344655'));
 a.push(p('M 36 69 L 48 83 58 68 62 71 51 90 45 90 32 72 Z','#f0dfc2'));
 a.push(p('M 31 73 L 34 72 44 91 40 96 34 96 25 78 Z','#d8a054'));
 a.push(p('M 64 72 L 69 76 59 96 55 96 52 91 61 73 Z','#d4a056'));
 a.push(ln('M 28 81 L 37 95 M 68 80 L 59 95',C.light,2));
 // large and symmetrical imperial shoulder epaulettes and dangling fringes
 a.push(p('M 6 77 L 14 70 28 67 37 73 31 81 15 85 5 82 Z','#8e642e'));
 a.push(p('M 8 75 L 17 69 29 69 34 74 29 78 14 82 7 81 Z',C.gold));
 a.push(ln('M 9 80 L 11 87 M 14 82 L 16 90 M 19 81 L 21 87 M 24 79 L 26 86 M 29 78 L 31 83',C.light,2));
 a.push(p('M 62 73 L 70 67 83 71 90 78 91 82 79 85 65 81 Z','#8e642e'));
 a.push(p('M 64 74 L 74 69 82 72 89 79 86 81 70 78 Z',C.gold));
 a.push(ln('M 68 80 L 67 87 M 73 81 L 72 89 M 78 82 L 77 90 M 83 82 L 84 87 M 88 81 L 89 86',C.light,2));
 // crimson diagonal sash, with sharp edged folds
 a.push(p('M 59 78 L 64 81 50 96 41 96 Z','#773c42'));
 a.push(ln('M 61 81 L 48 95',C.scarlet,2));
 // medals clustered on opposite side of sash, visible at 48
 a.push(r(26,85,4,3,C.cream)); a.push(r(31,84,4,3,'#5a8aa0')); a.push(r(36,85,4,3,C.cream));
 a.push(p('M 27 88 L 29 92 31 88 Z',C.light));a.push(p('M 32 87 L 34 91 36 87 Z','#e45c51'));a.push(p('M 37 88 L 39 92 41 88 Z','#81b6a8'));
 a.push(r(44,92,5,4,C.gold));a.push(r(45,92,2,1,C.white));
}
return a.join('');}
function face(gender){let a=[];
if(gender==='homme'){
 // auburn hair outside same skin region + heavy locks
 a.push(p('M 26 42 L 25 32 28 26 34 20 42 18 57 20 64 24 70 31 71 45 68 61 68 68 62 73 59 72 58 67 38 69 32 73 27 67 27 57 24 52 Z','#261f29'));
 a.push(p('M 27 36 L 32 26 41 22 54 22 65 28 67 38 61 36 57 31 46 31 41 34 34 32 30 41 Z','#794132'));
 a.push(p('M 28 34 L 36 25 46 22 55 24 61 28 48 28 43 31 34 31 Z','#c07a48'));
 a.push(p('M 58 26 L 67 32 68 43 64 42 63 32 Z','#55302c'));
 a.push(r(29,37,3,11,'#57302b'));a.push(r(65,36,4,11,'#492830'));
 // face and fixed pose
 a.push(p('M 32 38 L 38 32 58 32 65 38 66 52 63 65 58 73 48 77 39 72 33 64 30 52 Z','#2f242c'));
 a.push(p('M 33 39 L 39 34 57 34 63 39 63 53 59 64 55 70 48 73 40 69 35 61 33 52 Z','#d18b61'));
 a.push(p('M 35 41 L 40 35 46 35 42 48 38 57 37 63 34 58 Z','#e6a773'));
 a.push(p('M 58 35 L 62 40 62 53 57 65 51 70 53 60 55 48 Z','#985b49'));
 a.push(p('M 39 64 L 47 69 54 66 58 62 55 71 48 73 40 70 Z','#aa654f'));
 a.push(r(29,49,4,9,'#b47a5b'));a.push(r(63,49,4,9,'#8d5b4b'));
 a.push(r(31,51,2,4,'#e5a77d'));a.push(r(64,51,1,4,'#d48c62'));
 // eye brows distinct amber brow / small eyebrow scar, EXACT same every grade
 a.push(p('M 36 43 L 40 41 45 42 45 44 38 45 Z','#442b2b'));
 a.push(p('M 52 42 L 58 41 61 44 59 45 52 44 Z','#3b2930'));
 a.push(r(40,41,2,5,'#edb78a'));a.push(r(39,42,1,5,'#f8d19a')); // eyebrow scar
 a.push(r(36,47,11,2,'#764b46'));a.push(r(51,47,10,2,'#704846'));
 a.push(r(38,48,8,3,'#e0bf9c'));a.push(r(52,48,7,3,'#dcc09f'));
 a.push(r(41,48,2,3,'#304f59'));a.push(r(54,48,2,3,'#31505a'));
 a.push(r(42,48,1,1,'#fff3cf'));a.push(r(54,48,1,1,'#fff3cf'));
 a.push(p('M 48 49 L 46 57 45 59 48 62 53 60 52 58 49 59 Z','#ab624f'));
 a.push(r(46,61,7,2,'#f3ac79'));
 // stubbled moustache and angular chin short beard (no age change)
 a.push(p('M 37 63 L 44 62 47 65 50 65 54 62 59 64 55 67 50 68 45 68 40 67 Z','#603a36'));
 a.push(p('M 39 68 L 46 71 53 70 57 67 55 73 49 77 44 75 40 73 Z','#6f4139'));
 a.push(r(46,69,7,1,'#e7a174'));a.push(r(45,73,6,1,'#b1775c'));
 a.push(r(36,58,3,3,'#c77c58'));a.push(r(59,57,2,3,'#a9654f'));
 // tiny sun glints in hair, consistent
 a.push(r(32,27,5,2,'#dd955a'));a.push(r(43,23,8,2,'#e1a36d'));
}else{
 // dark curls and signature pale forelock, all unchanged across ranks
 a.push(p('M 21 91 L 22 70 24 49 22 38 26 28 35 20 43 18 54 19 64 24 70 31 74 44 72 55 74 68 78 90 69 92 64 77 57 79 35 77 29 94 Z','#1b2230'));
 a.push(p('M 25 44 L 25 34 31 27 38 23 51 21 61 25 67 33 68 44 63 42 58 35 46 31 39 35 30 44 Z','#333747'));
 a.push(p('M 25 43 L 29 31 35 27 43 25 52 25 46 29 38 31 31 39 Z','#60504f'));
 a.push(p('M 60 29 L 69 34 70 50 66 53 64 40 Z','#253141'));
 a.push(p('M 23 51 L 28 51 29 75 34 88 28 93 22 84 Z','#303447'));
 a.push(p('M 68 51 L 73 55 74 85 71 92 66 82 Z','#313848'));
 a.push(r(25,62,3,12,'#5c4a4b'));a.push(r(68,63,3,13,'#625153'));
 // face fixed
 a.push(p('M 32 37 L 39 31 57 31 64 38 65 52 60 67 54 74 47 77 39 72 34 64 30 52 Z','#2b2733'));
 a.push(p('M 34 38 L 41 33 56 33 62 39 62 52 58 65 53 70 47 73 40 70 36 63 33 51 Z','#cb8c6a'));
 a.push(p('M 35 40 L 42 34 47 34 41 52 37 59 35 59 Z','#eab288'));
 a.push(p('M 56 34 L 62 40 62 52 56 65 51 68 53 57 55 47 Z','#9a655b'));
 a.push(p('M 39 65 L 47 71 54 68 57 63 56 70 48 74 42 71 Z','#b87666'));
 a.push(r(30,49,4,9,'#bb8569'));a.push(r(63,49,3,9,'#9a695a'));
 a.push(r(31,51,2,5,'#edb793'));a.push(r(64,51,1,4,'#d99776'));
 // bright forelock / signature, unchanged below any hat
 a.push(p('M 54 25 L 61 26 64 31 60 35 53 37 48 42 42 42 48 35 52 31 Z','#c2baa0'));
 a.push(p('M 55 26 L 60 28 59 32 50 39 45 39 51 33 Z','#e4d8b0'));
 a.push(r(53,29,4,2,'#fff0c9'));
 // brows, eyes, gaze
 a.push(p('M 36 44 L 41 42 46 43 46 44 38 46 Z','#51383f'));
 a.push(p('M 52 43 L 57 42 61 45 60 46 52 44 Z','#523b43'));
 a.push(r(37,47,10,2,'#865a57'));a.push(r(51,47,10,2,'#825654'));
 a.push(r(39,48,7,3,'#ebc9aa'));a.push(r(53,48,6,3,'#e2be9e'));
 a.push(r(42,48,2,3,'#325365'));a.push(r(55,48,2,3,'#315264'));
 a.push(r(43,48,1,1,'#fff8de'));a.push(r(55,48,1,1,'#fff8de'));
 a.push(p('M 48 49 L 46 57 46 60 49 61 52 59 50 57 49 58 Z','#a7675a'));
 a.push(r(47,60,6,2,'#f0aa88'));
 a.push(p('M 41 64 L 46 64 49 65 54 63 57 64 52 68 47 69 43 67 Z','#8d4952'));
 a.push(r(46,66,7,1,'#e5a489'));
 a.push(r(57,61,2,2,'#452f3e')); // beauty mark on her left cheek
 a.push(r(38,57,3,2,'#e8a282'));
}
return a.join('');}
function hat(grade,gender){let a=[];
if(grade==='mousse'){
 // generous bandana, with a sail-like knot to right, sailor character not a pirate caricature
 a.push(p('M 25 33 L 29 26 38 23 49 22 59 23 68 27 70 35 67 38 59 33 48 31 39 31 29 37 Z','#382433'));
 a.push(p('M 27 31 L 35 25 47 23 60 25 67 29 68 34 59 31 49 29 38 29 29 35 Z','#a6453d'));
 a.push(p('M 30 28 L 43 24 53 24 58 26 46 26 36 29 Z','#e17c53'));
 a.push(ln('M 32 33 L 44 29 59 31',C.gold,1));
 a.push(p('M 64 30 L 73 30 76 35 73 41 66 37 Z','#a7473b'));
 a.push(p('M 71 38 L 79 45 76 52 70 48 67 41 Z','#d46f49'));
 a.push(r(72,44,3,4,'#8e3a37'));
 a.push(r(37,26,3,2,'#ffb276'));
}else if(grade==='capitaine'){
 // structured navy tricorn with brass trim and blue crown, brim end above face
 a.push(p('M 17 30 L 19 21 30 18 37 13 59 13 69 19 78 22 79 31 67 36 56 31 38 31 29 36 Z','#0c1727'));
 a.push(p('M 20 27 L 31 19 42 16 57 17 68 21 76 26 67 30 55 27 38 27 29 32 Z','#263c53'));
 a.push(p('M 21 27 L 35 19 40 18 35 25 26 29 Z','#456878'));
 a.push(p('M 22 31 L 37 27 57 27 73 30 68 33 55 30 39 30 28 34 Z','#a76c40'));
 a.push(ln('M 22 30 L 37 26 L 57 26 74 29',C.gold,2));
 a.push(p('M 46 17 L 51 17 54 21 51 24 46 24 43 21 Z',C.gold));
 a.push(r(48,19,2,3,C.deep));
 a.push(r(29,22,3,1,C.light));
}else if(grade==='amiral'){
 // dramatic naval bicorne with wing tips and discreet laurel insignia
 a.push(p('M 6 30 L 13 20 18 9 27 12 37 18 46 17 56 17 67 12 78 8 83 19 89 30 75 34 64 31 48 29 32 31 18 34 Z','#080f1c'));
 a.push(p('M 12 27 L 20 13 28 16 37 21 51 21 67 17 76 12 84 27 75 30 64 27 48 25 32 28 19 31 Z','#233345'));
 a.push(p('M 14 23 L 20 15 24 17 21 24 Z','#4d5352'));
 a.push(p('M 70 17 L 78 12 83 23 75 22 Z','#42535d'));
 a.push(ln('M 13 29 L 30 27 L 46 24 L 62 25 L 80 28',C.gold,2));
 a.push(ln('M 18 20 L 24 15 L 32 21 M 64 21 L 76 14',C.gold,1));
 a.push(p('M 43 17 L 51 17 56 22 52 27 46 26 41 22 Z',C.gold));
 a.push(p('M 47 19 L 51 19 53 22 50 25 46 23 Z',C.deep));
 a.push(r(20,24,4,2,'#e3b968'));a.push(r(75,23,4,2,'#e3b968'));
 // 4 stepped laurel leaves to signify senior rank
 a.push(p('M 36 22 L 39 19 42 21 41 23 Z',C.cream));a.push(p('M 56 21 L 59 19 62 21 60 23 Z',C.cream));
}
return a.join('');}
function portrait(gender,grade){return `<g shape-rendering="crispEdges">${seaUniform(grade,gender)}${face(gender)}${hat(grade,gender)}</g>`;}
const name=(g,grade)=>`grade_${g}_${grade}`;
const grades=['mousse','capitaine','amiral'], genders=['homme','femme'];
const wrap=(w,h,inside,viewBox=`0 0 ${w} ${h}`)=>`<svg xmlns="http://www.w3.org/2000/svg" width="${w}" height="${h}" viewBox="${viewBox}">${inside}</svg>\n`;
// six editable independent SVG sketches: only art, transparent canvas.
for(const g of genders) for(const grade of grades) emit(`${name(g,grade)}_etude.svg`,wrap(96,96,portrait(g,grade)));
// Design review board, intentional art study, larger than CYD.
function board(){let s=[];
s.push(`<defs><linearGradient id="ocean" x2="0" y2="1"><stop stop-color="#152b3e"/><stop offset="1" stop-color="#0c1b2d"/></linearGradient><pattern id="salt" width="30" height="31" patternUnits="userSpaceOnUse"><path d="M 2 6 h 9 M 20 24 h 6" stroke="#8e8775" stroke-opacity=".12"/></pattern></defs>`);
s.push(r(0,0,1160,835,'#e7d6b7'));s.push(r(10,10,1140,815,'#142739'));
s.push(r(18,18,1124,76,'#203747'));s.push(r(18,18,1124,4,C.gold));
s.push(`<text x="40" y="53" fill="#f3ddb8" font-family="serif" font-weight="bold" font-size="26" letter-spacing="2">PORTRAITS  /  GRADES</text>`);
s.push(`<text x="41" y="77" fill="#9bb8b7" font-family="sans-serif" font-size="14" letter-spacing="1">DEUX IDENTITÉS · UNE PROGRESSION DE MÊME ÂGE · PROPOSITION PO / PR 1</text>`);
for(let k=0;k<3;k++){
let x=30+k*375;
s.push(`<text x="${x+148}" y="116" text-anchor="middle" font-family="sans-serif" font-size="17" letter-spacing="3" font-weight="bold" fill="#e6bc76">${grades[k].toUpperCase()}</text>`);
}
for(let i=0;i<2;i++)for(let j=0;j<3;j++){
let x=30+j*375, y=130+i*311,g=genders[i],grade=grades[j];
s.push(r(x,y,349,293,'#0b1626'));s.push(r(x+3,y+3,343,287,'#415465'));
s.push(r(x+6,y+6,337,281,'url(#ocean)'));s.push(r(x+6,y+6,337,281,'url(#salt)'));
// copper chart frame, concentric rings, subtle thematic marine vignette
s.push(`<circle cx="${x+175}" cy="${y+146}" r="125" fill="none" stroke="#537482" stroke-opacity=".18" stroke-width="2"/>`);
s.push(`<circle cx="${x+175}" cy="${y+146}" r="106" fill="none" stroke="#537482" stroke-opacity=".12"/>`);
s.push(ln(`M ${x+21} ${y+41} L ${x+21} ${y+19} ${x+43} ${y+19}`,C.gold,2));
s.push(ln(`M ${x+306} ${y+274} L ${x+328} ${y+274} ${x+328} ${y+252}`,C.gold,2));
s.push(`<g transform="translate(${x+55} ${y+17}) scale(2.5)">${portrait(g,grade)}</g>`);
s.push(r(x+7,y+263,335,23,'#142739'));
s.push(`<text x="${x+174}" y="${y+279}" font-family="sans-serif" text-anchor="middle" fill="#e3cba2" font-size="12" letter-spacing="1">${name(g,grade)}</text>`);
}
s.push(`<text x="33" y="744" fill="#f1d39a" font-family="sans-serif" font-size="17" font-weight="bold">LIGNÉE HOMME</text>`);
s.push(`<text x="33" y="764" fill="#b3c2ba" font-family="sans-serif" font-size="14">Sourcil scarifié · regard vert d'eau · barbe courte rousse · mâchoire anguleuse</text>`);
s.push(`<text x="33" y="792" fill="#f1d39a" font-family="sans-serif" font-size="17" font-weight="bold">LIGNÉE FEMME</text>`);
s.push(`<text x="33" y="812" fill="#b3c2ba" font-family="sans-serif" font-size="14">Mèche ivoire · grain de beauté · regard vert d'eau · pommettes hautes</text>`);
s.push(`<text x="1136" y="810" text-anchor="end" fill="#9bb8b7" font-family="sans-serif" font-size="11">CANDIDATE · ON_DEVICE_UNVERIFIED</text>`);
return wrap(1160,835,s.join(''));}
emit('planche_six_concepts.svg',board());
// reference faces: no hat / common sailor structure; for identity gate only
function identities(){let s=[];s.push(r(0,0,800,244,'#132537'));s.push(r(12,12,776,220,'#1d3646'));s.push(`<text x="30" y="38" font-family="sans-serif" fill="#f3d49a" font-size="20" font-weight="bold" letter-spacing="1">DEUX VISAGES DE RÉFÉRENCE</text>`);
for(let i=0;i<2;i++){let x=38+i*382,g=genders[i];s.push(r(x,53,165,165,'#0d1a2c'));s.push(`<g transform="translate(${x+8} 60) scale(1.5)">${seaUniform('mousse',g)}${face(g)}</g>`);s.push(`<text x="${x+182}" y="97" font-family="sans-serif" fill="#ffd58b" font-size="18" font-weight="bold">${g.toUpperCase()}</text>`);let arr=g==='homme'?['Sourcil scarifié','Cheveux auburn','Barbe courte','Mâchoire marquée']:['Mèche ivoire','Grain de beauté','Cheveux bruns','Pommettes hautes'];for(let n=0;n<arr.length;n++)s.push(`<text x="${x+182}" y="${119+n*24}" font-family="sans-serif" fill="#c6d4c4" font-size="12">${arr[n]}</text>`);}return wrap(800,244,s.join(''));}
emit('deux_visages_references.svg',identities());
function nativeSheet(light=false){let s=[];let dark=light?'#e7dcc3':'#0c1b2a';let plate=light?'#f4ead6':'#162c40';let fg=light?'#24384a':'#f5dcad';s.push(r(0,0,320,240,dark));s.push(r(0,0,320,24,light?'#e4ca9a':'#284459'));s.push(`<text x="10" y="16" font-family="sans-serif" font-size="12" font-weight="bold" fill="${fg}">GRADES / ÉTUDE 48 PX</text>`);s.push(`<text x="310" y="16" font-family="sans-serif" text-anchor="end" font-size="9" fill="${fg}">${light?'FOND CLAIR':'FOND SOMBRE'}</text>`);
for(let j=0;j<3;j++)s.push(`<text x="${60+j*101}" y="40" font-size="9" text-anchor="middle" font-family="sans-serif" font-weight="bold" fill="${fg}">${grades[j].toUpperCase()}</text>`);
for(let i=0;i<2;i++)for(let j=0;j<3;j++){let x=10+j*101,y=45+i*90,g=genders[i];s.push(r(x,y,94,80,plate));s.push(ln(`M ${x} ${y+80} L ${x+94} ${y+80}`,light?'#beaa8c':'#496a70',1));s.push(`<g transform="translate(${x+23} ${y+4}) scale(0.5)">${portrait(g,grades[j])}</g>`);s.push(`<text x="${x+47}" y="${y+70}" text-anchor="middle" font-family="sans-serif" font-size="9" font-weight="bold" fill="${fg}">${g.toUpperCase()}</text>`);}
s.push(ln('M 8 227 L 312 227',light?'#998c76':'#576d78',1));s.push(`<text x="10" y="238" font-family="sans-serif" font-size="7.8" fill="${fg}">APERÇU CONCEPT · PAS UN EXPORT SPRITE 48x48 · NON TESTÉ SUR CYD</text>`);
return wrap(320,240,s.join(''));}
for(let light of [false,true]){let n=light?'clair':'sombre';let svg=nativeSheet(light);emit(`apercu_${n}_320x240.svg`,svg);}
console.log(check?'CHECK PASS 10 SVG':'GENERATED 10 SVG',fs.readdirSync(out));

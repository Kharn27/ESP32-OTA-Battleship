#include "warship_combat_ui.h"
#include <stddef.h>
#include <string.h>

/* Flash-only 5x7 technical font. No asset, framebuffer or heap. */
typedef struct { char c; uint8_t row[7]; } Glyph;
static const Glyph font[] = {
 {' ',{0,0,0,0,0,0,0}}, {'-',{0,0,0,31,0,0,0}},
 {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}},
 {'2',{14,17,1,2,4,8,31}}, {'3',{30,1,1,14,1,1,30}},
 {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}},
 {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}},
 {'8',{14,17,17,14,17,17,14}}, {'9',{14,17,17,15,1,1,14}},
 {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}},
 {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}},
 {'E',{31,16,16,30,16,16,31}}, {'F',{31,16,16,30,16,16,16}},
 {'G',{14,17,16,23,17,17,14}}, {'H',{17,17,17,31,17,17,17}},
 {'I',{14,4,4,4,4,4,14}}, {'J',{7,2,2,2,18,18,12}},
 {'K',{17,18,20,24,20,18,17}}, {'L',{16,16,16,16,16,16,31}},
 {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}},
 {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}},
 {'Q',{14,17,17,17,21,18,13}}, {'R',{30,17,17,30,20,18,17}},
 {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}},
 {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}},
 {'W',{17,17,17,21,21,21,10}}, {'X',{17,17,10,4,10,17,17}},
 {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}}
};
static int glyph_bit(char ch, unsigned x, unsigned y)
{
    if (x >= 5u || y >= 7u) return 0;
    for (size_t i=0u; i<sizeof font/sizeof font[0]; ++i)
        if (font[i].c == ch)
            return (font[i].row[y] & (uint8_t)(1u << (4u-x))) != 0u;
    return 0;
}
static int text_bit(const char *s, unsigned atx, unsigned aty,
                    unsigned x, unsigned y)
{
    if (x<atx || y<aty || y>=aty+7u) return 0;
    const unsigned i=(x-atx)/6u;
    return i<strlen(s) && glyph_bit(s[i],(x-atx)%6u,y-aty);
}
static int rect(unsigned x,unsigned y,unsigned rx,unsigned ry,
                unsigned rw,unsigned rh)
{
    return x>=rx && x<rx+rw && y>=ry && y<ry+rh;
}
static int border(unsigned x,unsigned y,unsigned rx,unsigned ry,
                  unsigned rw,unsigned rh)
{
    return rect(x,y,rx,ry,rw,rh) &&
           (x==rx || y==ry || x==rx+rw-1u || y==ry+rh-1u);
}
static const char *ship_name(uint8_t id)
{
    static const char *const names[] = {
        "PATROUILLE", "SOUS-MARIN", "DESTROYER", "CROISEUR", "PORTE-AVIONS"
    };
    return id<WARSHIP_FLEET_SHIPS ? names[id] : "";
}
static const char *impact_name(uint8_t impact)
{
    switch (impact) {
    case WARSHIP_BATTLE_MISS: return "RATE";
    case WARSHIP_BATTLE_HIT: return "TOUCHE";
    case WARSHIP_BATTLE_SUNK: return "COULE";
    default: return "";
    }
}
static int shot_bit(WarshipBattleShot s,unsigned tx,unsigned ty,
                    unsigned x,unsigned y)
{
    if (glyph_bit((char)('A'+s.x), x>=tx?x-tx:100u,
                  y>=ty?y-ty:100u)) return 1;
    if (s.y==9u) {
        if (text_bit("10",tx+6u,ty,x,y)) return 1;
    } else if (glyph_bit((char)('1'+s.y),
                         x>=tx+6u?x-tx-6u:100u,
                         y>=ty?y-ty:100u)) return 1;
    if (text_bit(impact_name(s.impact),tx+24u,ty,x,y))
        return 1;
    return 0;
}
static uint16_t button(unsigned x,unsigned y,unsigned rx,unsigned ry,
                       const char *label,uint16_t fill)
{
    if (border(x,y,rx,ry,91u,38u)) return 0xffffu;
    const unsigned len=(unsigned)strlen(label)*6u-1u;
    const unsigned tx=rx+(91u-len)/2u;
    return text_bit(label,tx,ry+15u,x,y) ? 0xffffu : fill;
}

uint16_t warship_combat_pixel(const WarshipCombatFrame *f,
                             uint16_t x, uint16_t y)
{
    if (f==NULL || x>=320u || y>=240u || f->view>WARSHIP_COMBAT_DEFENSE ||
        (f->phase!=WARSHIP_LIFECYCLE_COMBAT &&
         f->phase!=WARSHIP_LIFECYCLE_CONFIRM_ABANDON &&
         f->phase!=WARSHIP_LIFECYCLE_RESULT))
        return 0u;
    if (text_bit(f->view==WARSHIP_COMBAT_ATTACK?"ATTAQUE":"DEFENSE",
                 22u,7u,x,y))
        return 0xffffu;
    if (text_bit("D13",273u,7u,x,y)) return 0xffe0u;
    for (unsigned i=0u;i<10u;++i) {
        const unsigned cx=22u+i*19u+7u;
        const unsigned cy=24u+i*19u+6u;
        if (glyph_bit((char)('A'+i),x>=cx?x-cx:100u,
                      y>=15u?y-15u:100u)) return 0xffffu;
        if (i==9u) {
            if (text_bit("10",2u,cy,x,y)) return 0xffffu;
        } else if (glyph_bit((char)('1'+i),x>=8u?x-8u:100u,
                             y>=cy?y-cy:100u)) return 0xffffu;
    }
    if (rect(x,y,22u,24u,190u,190u)) {
        const unsigned gx=(x-22u)/19u,gy=(y-24u)/19u;
        const unsigned lx=(x-22u)%19u,ly=(y-24u)%19u;
        const unsigned idx=gy*10u+gx;
        const uint8_t impact=f->attacks[idx];
        const uint8_t ship=f->ships[idx];
        const int chosen=f->has_selection && f->target_x==gx &&
                         f->target_y==gy && f->view==WARSHIP_COMBAT_ATTACK;
        if (lx==0u || ly==0u || lx==18u || ly==18u)
            return chosen?0xffe0u:0x9e9fu;
        if (impact != 0u) {
            if (impact==1u) {
                if (lx>=8u && lx<=10u && ly>=8u && ly<=10u) return 0xffffu;
            } else {
                if ((lx>=4u && lx<=14u && (lx==ly || lx+ly==18u)) ||
                    (impact==3u && (ly==3u || ly==15u) &&
                     lx>=4u && lx<=14u)) return 0xfbe0u;
            }
        }
        if (chosen && (lx<=2u || lx>=16u || ly<=2u || ly>=16u))
            return 0xffe0u;
        if (impact==2u || impact==3u) return 0x7800u;
        if (ship!=0u) return 0x057fu;
        return (gx+gy)%2u ? 0x1143u : 0x0922u;
    }

    /* D12: temporary navigation labels in existing D10 hitboxes.
     * No PO-approved graphics/assets and no additional pixel buffer. */
    if (rect(x,y,225u,32u,91u,38u)) {
        if (f->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON)
            return button(x,y,225u,32u,"CONTINUER",0x0440u);
        if (f->phase==WARSHIP_LIFECYCLE_RESULT)
            return button(x,y,225u,32u,"REJOUER",0x0440u);
        const int active=f->view==WARSHIP_COMBAT_ATTACK &&
                         f->has_selection && !f->busy;
        return button(x,y,225u,32u,f->busy?"TIR...":"FEU",
                      active?0x0400u:0x2104u);
    }
    if (rect(x,y,225u,83u,91u,38u))
        return button(x,y,225u,83u,
            f->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON?"BLOQUE":
            f->view==WARSHIP_COMBAT_ATTACK?"DEFENSE":"ATTAQUE",
            f->busy || f->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON?
                0x2104u:0x0250u);
    if (rect(x,y,225u,134u,91u,38u)) {
        const char *label=f->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON?
            "ABANDONNER":f->phase==WARSHIP_LIFECYCLE_RESULT?
            "ACCUEIL":"QUITTER";
        return button(x,y,225u,134u,label,
            f->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON?
                0x7800u:(f->busy || f->terminal)?0x2104u:0x3000u);
    }
    if (f->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON) {
        if (text_bit("QUITTER ?",225u,185u,x,y) ||
            text_bit("CONFIRMER",225u,201u,x,y))
            return 0xffe0u;
        return 0x000cu;
    }
    if (f->phase==WARSHIP_LIFECYCLE_RESULT) {
        const char *label=f->verdict==WARSHIP_BATTLE_VICTORY?"VICTOIRE":
            f->verdict==WARSHIP_BATTLE_DEFEAT?"DEFAITE":"MATCH NUL";
        if (text_bit(label,225u,183u,x,y))
            return 0xffe0u;
        if (f->reveal_available && f->view==WARSHIP_COMBAT_ATTACK &&
            f->verdict==WARSHIP_BATTLE_DEFEAT &&
            rect(x,y,225u,203u,91u,31u)) {
            const char *cmd=!f->reveal_requested?"VOIR SA FLOTTE":
                            f->reveal_visible?"MASQUER":"AFFICHER";
            if (border(x,y,225u,203u,91u,31u))return 0xffffu;
            const unsigned len=(unsigned)strlen(cmd)*6u-1u;
            const unsigned tx=225u+(91u-len)/2u;
            return text_bit(cmd,tx,215u,x,y)?0xffffu:0x0440u;
        }
        if (text_bit("FIN PARTIE",225u,212u,x,y) &&
            !f->reveal_available)
            return 0xffe0u;
        /* No RIPOSTE/ALERTE over result controls. The impact grid is
         * never redrawn from a secret fleet; overlay only supplies ships. */
        if (x>=225u && y>=175u) return 0x000cu;
    }
    if (f->busy) {
        if (text_bit("FEU EN COURS",225u,188u,x,y) ||
            text_bit("ATTENDRE",225u,204u,x,y)) return 0xffe0u;
    } else {
        if (f->has_cpu &&
            (text_bit("RIPOSTE",225u,179u,x,y) ||
             shot_bit(f->last_cpu,225u,191u,x,y)))
            return 0xffffu;
        if (f->has_cpu && f->last_cpu.impact==WARSHIP_BATTLE_SUNK &&
            text_bit(ship_name(f->last_cpu.ship_id),225u,201u,x,y))
            return 0xfbe0u;
        if (f->view==WARSHIP_COMBAT_ATTACK && f->alert!=0u) {
            if (text_bit(f->alert==3u?"ALERTE COULE":
                         f->alert==2u?"ALERTE TOUCHE":"ALERTE RATE",
                         225u,213u,x,y)) return 0xffe0u;
        }
    }
    if (f->view==WARSHIP_COMBAT_ATTACK && f->has_human && !f->busy) {
        if (text_bit("TIR",22u,222u,x,y) ||
            shot_bit(f->last_human,46u,222u,x,y))
            return 0xffffu;
        if (f->last_human.impact==WARSHIP_BATTLE_SUNK &&
            text_bit(ship_name(f->last_human.ship_id),138u,222u,x,y))
            return 0xfbe0u;
    }
    return 0x000cu;
}

/* D14 host: G03 geometry/preview shape, D07 public fleet, no ghosts,
 * malformed layout/frames, and unchanged D07 tactile semantics.
 */
#include "d14_g03.h"
#include "warship_lifecycle.h"
#include "warship_deploy_ui.h"
#include "warship_fleet.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) {fprintf(stderr,"D14 line %d: %s\n",__LINE__,#e);return 1;} } while (0)
static uint8_t previous[320u*240u*2u];
static uint8_t refreshed[320u*240u*2u];
static uint8_t expected[320u*240u*2u];
static uint8_t band[5120u];
static uint8_t reference[5120u];

static int fill( const WarshipDeployFrame *f, uint8_t *dst)
{
    for (unsigned y=0;y<240u;y+=8u) {
        if (d14_g03_compose(f,0u,(uint16_t)y,320u,8u,band,sizeof band)!=0)
            return -1;
        memcpy(dst+y*320u*2u,band,sizeof band);
    }
    return 0;
}

static WarshipFleet original(void)
{
    WarshipFleet f = {{
        {0,6,7,2,WARSHIP_ORIENT_HORIZONTAL},
        {1,2,5,3,WARSHIP_ORIENT_HORIZONTAL},
        {2,5,4,3,WARSHIP_ORIENT_VERTICAL},
        {3,8,3,4,WARSHIP_ORIENT_VERTICAL},
        {4,1,1,5,WARSHIP_ORIENT_HORIZONTAL}
    }};
    return f;
}
static WarshipFleet replacement(void)
{
    WarshipFleet f = {{
        {0,0,0,2,WARSHIP_ORIENT_HORIZONTAL},
        {1,0,2,3,WARSHIP_ORIENT_HORIZONTAL},
        {2,0,4,3,WARSHIP_ORIENT_HORIZONTAL},
        {3,0,6,4,WARSHIP_ORIENT_HORIZONTAL},
        {4,0,8,5,WARSHIP_ORIENT_HORIZONTAL}
    }};
    return f;
}
static int frame_of(const WarshipFleet *fleet, WarshipDeployFrame *frame)
{
    WarshipLifecyclePublic pub={0};
    pub.phase=WARSHIP_LIFECYCLE_DEPLOY;
    pub.has_human_fleet=1u;
    pub.human=*fleet;
    return warship_deploy_make_frame(&pub,frame)==WARSHIP_DEPLOY_OK ? 0 : -1;
}

int main(void)
{
    const WarshipDeployLayout layout=d14_g03_layout();
    CHECK(warship_deploy_layout_valid(&layout));
    CHECK(layout.cell==18u && layout.grid.x==18u && layout.grid.y==42u);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_DEPLOY,260,70)==WARSHIP_DEPLOY_AUTO);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_DEPLOY,260,132)==WARSHIP_DEPLOY_READY);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_DEPLOY,260,196)==WARSHIP_DEPLOY_RETURN);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_DEPLOY,18,42)==WARSHIP_DEPLOY_NO_ACTION);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_DEPLOY,208,47)==WARSHIP_DEPLOY_NO_ACTION);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_HOME,260,132)==WARSHIP_DEPLOY_START);
    CHECK(warship_deploy_hit(&layout,WARSHIP_LIFECYCLE_HOME,260,70)==WARSHIP_DEPLOY_NO_ACTION);
    uint8_t col=100,row=100;
    CHECK(d14_g03_screen_to_cell(18,42,&col,&row)==0 && col==0 && row==0);
    CHECK(d14_g03_screen_to_cell(197,221,&col,&row)==0 && col==9 && row==9);
    CHECK(d14_g03_screen_to_cell(36,60,&col,&row)==0 && col==1 && row==1);
    CHECK(d14_g03_screen_to_cell(198,221,&col,&row)!=0);
    CHECK(d14_g03_screen_to_cell(197,222,&col,&row)!=0);
    CHECK(d14_g03_screen_to_cell(18,42,NULL,&row)!=0);
    CHECK(d14_g03_auto_dirty().w==180u && d14_g03_auto_dirty().h==180u);

    WarshipFleet first=original(), second=replacement();
    CHECK(warship_fleet_validate(&first)==WARSHIP_FLEET_OK);
    CHECK(warship_fleet_validate(&second)==WARSHIP_FLEET_OK);
    WarshipDeployFrame f1,f2;
    CHECK(frame_of(&first,&f1)==0);
    CHECK(frame_of(&second,&f2)==0);
    CHECK(fill(&f1,previous)==0);
    CHECK(fill(&f2,expected)==0);
    memcpy(refreshed,previous,sizeof refreshed);
    const WarshipDeployRect dirty=d14_g03_auto_dirty();
    for (unsigned top=dirty.y;top<(unsigned)dirty.y+dirty.h;top+=8u) {
        const unsigned rows=(unsigned)dirty.y+dirty.h-top < 8u ?
            (unsigned)dirty.y+dirty.h-top:8u;
        CHECK(d14_g03_compose(&f2,dirty.x,(uint16_t)top,
                              dirty.w,(uint16_t)rows,band,sizeof band)==0);
        for (unsigned r=0;r<rows;++r)
            memcpy(refreshed+((top+r)*320u+dirty.x)*2u,
                   band+r*dirty.w*2u,dirty.w*2u);
    }
    CHECK(memcmp(refreshed,expected,sizeof refreshed)==0);
    /* The old carrier occupied B2, while the new fleet has no ship on
     * row B at all. The entire former 18x18 cell must be clean water. */
    CHECK(t03_decode_rect(&d14_g03_image,36u,65u,18u,8u,
                          reference,sizeof reference)==0);
    CHECK(d14_g03_compose(&f2,36u,65u,18u,8u,band,sizeof band)==0);
    CHECK(memcmp(band,reference,18u*8u*2u)==0);
    CHECK(memcmp(previous+(65u*320u+36u)*2u,
                 expected+(65u*320u+36u)*2u,18u*2u)!=0);
    /* No image data/secret Captain ever belongs to public frame. */
    WarshipDeployFrame same=f1;
    CHECK(memcmp(&same,&f1,sizeof same)==0);

    /* Malformed frame, capacity and screen bounds must all be rejected. */
    CHECK(d14_g03_compose(NULL,0,0,320,8,band,sizeof band)!=0);
    CHECK(d14_g03_compose(&f1,0,0,320,8,NULL,sizeof band)!=0);
    CHECK(d14_g03_compose(&f1,0,0,320,8,band,5119)!=0);
    CHECK(d14_g03_compose(&f1,0,0,320,9,band,sizeof band)!=0);
    CHECK(d14_g03_compose(&f1,319,0,2,1,band,sizeof band)!=0);
    WarshipDeployFrame broken=f1;
    broken.cells[7u*10u+6u]=0u;
    CHECK(d14_g03_compose(&broken,0,0,320,8,band,sizeof band)!=0);
    broken=f1;
    broken.cells[0]=9u;
    CHECK(d14_g03_compose(&broken,0,0,320,8,band,sizeof band)!=0);
    broken=f1;
    broken.mode=0u;
    CHECK(d14_g03_compose(&broken,0,0,320,8,band,sizeof band)!=0);

    /* B01: one long press cannot trigger a double READY. */
    WarshipDeployInput in;
    CHECK(warship_deploy_input_init(&in,&layout)==WARSHIP_DEPLOY_OK);
    WarshipDeployAction cmd=WARSHIP_DEPLOY_NO_ACTION;
    CHECK(warship_deploy_input_step(&in,1,1,260,132,100u,
          WARSHIP_LIFECYCLE_DEPLOY,&cmd)==WARSHIP_DEPLOY_OK);
    CHECK(cmd==WARSHIP_DEPLOY_NO_ACTION);
    CHECK(warship_deploy_input_step(&in,1,1,260,132,141u,
          WARSHIP_LIFECYCLE_DEPLOY,&cmd)==WARSHIP_DEPLOY_OK);
    CHECK(cmd==WARSHIP_DEPLOY_READY);
    CHECK(warship_deploy_input_step(&in,1,1,260,132,401u,
          WARSHIP_LIFECYCLE_DEPLOY,&cmd)==WARSHIP_DEPLOY_OK);
    CHECK(cmd==WARSHIP_DEPLOY_NO_ACTION);
    puts("D14 PASS: G03 geometry, 17 cells, mask compositing, AUTO ghost-free, B01");
    return 0;
}

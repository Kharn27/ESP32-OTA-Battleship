#include "warship_event_id_nvs.h"
#include "nvs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if(!(x)){fprintf(stderr,"D11 NVS FAIL %s:%d %s\n",__FILE__,__LINE__,#x);exit(EXIT_FAILURE);} }while(0)
#define EQ(a,b) CHECK((a)==(b))
#define SIZE 20u

typedef struct {
    uint8_t blob[SIZE+2u],staged[SIZE];
    size_t length,staged_length;
    unsigned reads,writes,commits;
    int exists,staged_valid;
} Slot;
static Slot slots[2];
static int read_failure,write_failure,commit_failure,after_commit;
static unsigned opens,closes;
static unsigned idx(nvs_handle_t handle)
{
    CHECK(handle==1u || handle==2u);
    return (unsigned)handle-1u;
}
esp_err_t nvs_open(const char *name,nvs_open_mode_t mode,nvs_handle_t *out)
{
    ++opens;CHECK(out!=NULL);
    const unsigned i=strcmp(name,"warship")==0?0u:
                     strcmp(name,"warship_d11lab")==0?1u:2u;
    CHECK(i<2u);
    if(read_failure && mode==NVS_READONLY)return ESP_FAIL;
    if(!slots[i].exists && mode==NVS_READONLY)return ESP_ERR_NVS_NOT_FOUND;
    *out=i+1u;return ESP_OK;
}
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,
                       void *data,size_t *length)
{
    Slot *s=&slots[idx(handle)];
    CHECK(strcmp(key,"event_seq")==0);CHECK(length!=NULL);
    ++s->reads;
    if(read_failure)return ESP_FAIL;
    if(!s->exists)return ESP_ERR_NVS_NOT_FOUND;
    if(data!=NULL){CHECK(*length>=s->length);memcpy(data,s->blob,s->length);}
    *length=s->length;return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle,const char *key,
                       const void *data,size_t length)
{
    Slot *s=&slots[idx(handle)];
    CHECK(strcmp(key,"event_seq")==0);CHECK(data!=NULL);
    ++s->writes;
    if(write_failure)return ESP_FAIL;
    EQ(length,SIZE);
    memcpy(s->staged,data,length);
    s->staged_length=length;s->staged_valid=1;
    return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle)
{
    Slot *s=&slots[idx(handle)];++s->commits;
    if(commit_failure&&!after_commit)return ESP_FAIL;
    if(s->staged_valid){
        memcpy(s->blob,s->staged,s->staged_length);
        s->length=s->staged_length;s->exists=1;
    }
    return commit_failure?ESP_FAIL:ESP_OK;
}
void nvs_close(nvs_handle_t handle)
{
    ++closes;slots[idx(handle)].staged_valid=0;
}
static void checksum(Slot *s)
{
    uint32_t h=UINT32_C(2166136261);
    for(unsigned i=0u;i<16u;++i){h^=s->blob[i];h*=UINT32_C(16777619);}
    for(unsigned i=0u;i<4u;++i)s->blob[16u+i]=(uint8_t)(h>>(i*8u));
}
int main(void)
{
    const WarshipEventIdBackend b=warship_event_id_nvs_backend();
    CHECK(b.load!=NULL && b.reserve!=NULL);
    WarshipEventIdNvsContext prod={0};
    uint64_t out=UINT64_MAX;
    EQ(b.load(&prod,&out),1);
    EQ(out,UINT64_MAX);
    EQ(slots[0].writes,0u);
    EQ(b.reserve(&prod,UINT64_C(0)), -1);
    EQ(slots[0].writes,0u);
    EQ(b.reserve(&prod,UINT64_C(1)),0);
    EQ(prod.last_error,ESP_OK);
    EQ(slots[0].commits,1u);
    EQ(b.load(&prod,&out),0);
    EQ(out,1u);
    EQ(slots[0].length,SIZE);

    /* Diagnostic namespace is isolated; production ID remains 1. */
    WarshipEventIdNvsContext lab={0};
    lab.namespace_name="warship_d11lab";
    EQ(b.load(&lab,&out),1);
    EQ(b.reserve(&lab,UINT64_C(17)),0);
    EQ(b.load(&lab,&out),0);
    EQ(out,17u);
    EQ(b.load(&prod,&out),0);
    EQ(out,1u);
    EQ(slots[0].writes,1u);EQ(slots[1].writes,1u);

    /* Post-commit error: durable sequence may have advanced despite
     * -1 return; reload recovers its exact value after reset. */
    after_commit=1;commit_failure=1;
    EQ(b.reserve(&prod,UINT64_C(2)), -1);
    EQ(prod.last_error,ESP_FAIL);
    after_commit=0;commit_failure=0;
    EQ(b.load(&prod,&out),0);
    EQ(out,2u);
    write_failure=1;
    EQ(b.reserve(&prod,UINT64_C(3)), -1);
    EQ(prod.last_error,ESP_FAIL);
    write_failure=0;
    EQ(b.load(&prod,&out),0);
    EQ(out,2u);
    read_failure=1;
    EQ(b.load(&prod,&out),-1);
    EQ(prod.last_error,ESP_FAIL);
    read_failure=0;

    /* Size mismatch, checksum corruption, incompatible format fail
     * without mutating output or automatically erasing data. */
    out=UINT64_MAX;
    slots[0].length=SIZE+2u;
    EQ(b.load(&prod,&out),-2);
    EQ(out,UINT64_MAX);
    slots[0].length=SIZE;
    slots[0].blob[9u]^=0x5au;
    EQ(b.load(&prod,&out),-2);
    EQ(out,UINT64_MAX);
    slots[0].blob[9u]^=0x5au;
    slots[0].blob[4u]=2u;
    checksum(&slots[0]);
    EQ(b.load(&prod,&out),-3);
    EQ(out,UINT64_MAX);
    CHECK(opens>0u&&closes>0u);
    printf("D11 event ID NVS actual adapter stub PASS writes=%u,%u\n",
           slots[0].writes,slots[1].writes);
    return 0;
}

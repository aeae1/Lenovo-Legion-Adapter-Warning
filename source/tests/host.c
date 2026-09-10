/* Host-only fault injection. Never built into the delivered EFI binaries. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define HOST_TEST
#include "../core.h"
static UINT8 *fixture,*live;
static UINT64 regs4,pte_flags,attrs;
static int five,leaf_level,attr_present,query_fail,clear_fail,restore_fail,
 clear_no_effect,clear_query_fail,restore_query_fail,restore_lies,write_fail,
 trigger_present,duplicate,absent,clear_calls,restore_calls,writes,queries;
static EFI_STATUS trigger_error;
static EFI_GUID path_guid;
static UINT8 path[24];
static EFI_LOADED_IMAGE_PROTOCOL li;
static EFI_HANDLE handle=(void *)2;
static int tests;
static UINT64 cr0(void){return (1ULL<<31)|(1ULL<<16);}
static UINT64 cr3(void){return 0x1000;}
static UINT64 cr4(void){return (1ULL<<5)|regs4|(five?(1ULL<<12):0);}
static void serialize(void){}
static UINT64 read_entry(UINT64 table,UINTN index){
 (void)index;int level=(five?4:3)-(int)(table/4096-1);
 if(level==leaf_level){UINT64 size=1ULL<<(12+9*level);return ((UINTN)(live+RVA)&~(size-1))|pte_flags|(level?128:0);}
 return (table+4096)|3;
}
static void store_byte(volatile UINT8 *p,UINT8 b){writes++;assert(p==live+RVA);assert(pte_flags&2);if(!write_fail)*p=b;}
static EFI_STATUS EFIAPI getmap(UINTN *n,EFI_MEMORY_DESCRIPTOR *d,UINTN *key,UINTN *ds,UINT32 *ver){
 *ds=sizeof(*d);*ver=1;*key=1;
 if(!d||*n<sizeof(*d)){*n=sizeof(*d);return EFI_BUFFER_TOO_SMALL;}
 *n=sizeof(*d);memset(d,0,sizeof(*d));d->Type=EfiBootServicesData;d->NumberOfPages=1ULL<<36;return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI alloc(UINT32 type,UINTN n,void **p){(void)type;*p=malloc(n);return *p?EFI_SUCCESS:EFI_OUT_OF_RESOURCES;}
static EFI_STATUS EFIAPI freemem(void *p){free(p);return EFI_SUCCESS;}
static EFI_STATUS EFIAPI handles(UINT32 type,EFI_GUID *g,void *key,UINTN *n,EFI_HANDLE **h){
 (void)type;(void)g;(void)key;*n=absent?0:duplicate?2:1;*h=malloc(2*sizeof(void *));(*h)[0]=handle;(*h)[1]=(void *)3;return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI openproto(EFI_HANDLE h,EFI_GUID *g,void **out,EFI_HANDLE a,EFI_HANDLE c,UINT32 flag){
 (void)h;(void)g;(void)a;(void)c;assert(flag==EFI_OPEN_PROTOCOL_GET_PROTOCOL);*out=&li;return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI getattrs(EFI_MEMORY_ATTRIBUTE_PROTOCOL *p,UINT64 a,UINT64 n,UINT64 *out){
 (void)p;assert(a==((UINTN)(live+RVA)&~4095ULL));assert(n==4096);queries++;
 if(query_fail||(clear_query_fail&&clear_calls&&!restore_calls)||(restore_query_fail&&restore_calls))return EFI_DEVICE_ERROR;
 *out=attrs;return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI clearattrs(EFI_MEMORY_ATTRIBUTE_PROTOCOL *p,UINT64 a,UINT64 n,UINT64 bits){
 (void)p;(void)a;assert(n==4096&&bits==EFI_MEMORY_RO);clear_calls++;
 if(clear_fail)return EFI_WRITE_PROTECTED;
 if(!clear_no_effect){attrs&=~EFI_MEMORY_RO;pte_flags|=2;}return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI setattrs(EFI_MEMORY_ATTRIBUTE_PROTOCOL *p,UINT64 a,UINT64 n,UINT64 bits){
 (void)p;(void)a;assert(n==4096&&bits==EFI_MEMORY_RO);restore_calls++;
 if(restore_fail)return EFI_DEVICE_ERROR;
 attrs|=EFI_MEMORY_RO;if(!restore_lies)pte_flags&=~2ULL;return EFI_SUCCESS;
}
static EFI_MEMORY_ATTRIBUTE_PROTOCOL ap={getattrs,setattrs,clearattrs};
static EFI_STATUS EFIAPI locate(EFI_GUID *g,void *key,void **p){
 (void)key;
 if(same(g,&trigger_guid,16)){*p=NULL;return trigger_error?trigger_error:trigger_present?EFI_SUCCESS:EFI_NOT_FOUND;}
 assert(same(g,&EFI_MEMORY_ATTRIBUTE_PROTOCOL_GUID,16));*p=attr_present?&ap:NULL;return attr_present?EFI_SUCCESS:EFI_NOT_FOUND;
}
static EFI_TPL EFIAPI raise(EFI_TPL t){assert(t==TPL_NOTIFY);return TPL_APPLICATION;}
static void EFIAPI lower(EFI_TPL t){assert(t==TPL_APPLICATION);}
static EFI_BOOT_SERVICES bs;
static void reset(void){
 memcpy(live,fixture,IMAGE_SIZE);zero(&li,sizeof(li));zero(&bs,sizeof(bs));
 zero(path,sizeof(path));path[0]=4;path[1]=6;path[2]=20;memcpy(path+4,&module_guid,16);path[20]=0x7f;path[21]=0xff;path[22]=4;
 li.FilePath=(void *)path;li.ImageBase=live;li.ImageSize=IMAGE_SIZE;li.ImageCodeType=EfiBootServicesCode;
 regs4=0;pte_flags=3;attrs=0;five=0;leaf_level=0;attr_present=0;
 query_fail=clear_fail=restore_fail=clear_no_effect=clear_query_fail=restore_query_fail=restore_lies=write_fail=trigger_present=duplicate=absent=0;
 clear_calls=restore_calls=writes=queries=0;trigger_error=0;
 ap.GetMemoryAttributes=getattrs;ap.ClearMemoryAttributes=clearattrs;ap.SetMemoryAttributes=setattrs;
 bs.GetMemoryMap=getmap;bs.AllocatePool=alloc;bs.FreePool=freemem;bs.LocateHandleBuffer=handles;
 bs.OpenProtocol=openproto;bs.LocateProtocol=locate;bs.RaiseTPL=raise;bs.RestoreTPL=lower;gBS=&bs;gSelf=(void *)1;
}
static RESULT check(const char *name,int mode,int early,UINTN expected,int expected_writes){
 RESULT r;run(&r,mode,early);
 if(r.reason!=expected||writes!=expected_writes){fprintf(stderr,"FAIL %s reason=%llu wanted=%llu writes=%d wanted=%d\n",name,r.reason,expected,writes,expected_writes);exit(1);}
 printf("PASS %s\n",name);tests++;return r;
}
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);fixture=malloc(IMAGE_SIZE);assert(fread(fixture,1,IMAGE_SIZE,f)==IMAGE_SIZE);fclose(f);
 live=aligned_alloc(4096,0x6000);assert(live);
 RESULT r;
 reset();r=check("diagnostic does not write",0,0,OK,0);assert(r.ready&&!r.patched);
 reset();r=check("writable patch changes one byte",1,1,OK,1);assert(r.verified);assert(live[RVA]==0xc3);live[RVA]=0x48;assert(!memcmp(live,fixture,IMAGE_SIZE));
 reset();absent=1;check("no target",1,1,TARGET_MISSING,0);
 reset();duplicate=1;check("duplicate module handles",1,1,TARGET_AMBIGUOUS,0);
 reset();path[4]^=1;check("wrong firmware GUID",1,1,TARGET_MISSING,0);
 reset();path[2]=3;check("malformed device path",1,1,TARGET_MISSING,0);
 reset();li.FilePath=NULL;check("GUID unavailable",1,1,TARGET_MISSING,0);
 reset();live[RVA+9]^=1;check("wrong callback bytes",1,1,IDENTITY_ERROR,0);
 reset();live[TEXT_RVA+10]^=1;check("changed code outside signature",1,1,IDENTITY_ERROR,0);
 reset();live[0x3c]=0xff;check("invalid PE offset",1,1,IDENTITY_ERROR,0);
 reset();li.ImageSize=0x1000;check("undersized image",1,1,IDENTITY_ERROR,0);
 reset();live[RVA]=0xc3;check("already patched or older helper",1,1,IDENTITY_ERROR,0);
 reset();trigger_present=1;check("late automatic driver",1,1,TOO_LATE,0);
 reset();trigger_present=1;check("manual mechanics permitted late",1,0,OK,1);
 reset();trigger_error=EFI_DEVICE_ERROR;check("unknown trigger state",1,1,TRIGGER_QUERY_ERROR,0);
 reset();pte_flags=1;check("RO without protocol",1,1,PERMISSION_BLOCKED,0);
 reset();attr_present=1;query_fail=1;check("failed query on writable page",1,1,ATTR_QUERY_ERROR,0);assert(clear_calls==0);
 reset();attr_present=1;query_fail=1;pte_flags=1;check("failed query on RO page",1,1,ATTR_QUERY_ERROR,0);assert(clear_calls==0);
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;r=check("RO clear write restore",1,1,OK,1);assert(r.cleared_ro&&r.restored_ro&&!(pte_flags&2)&&attrs==EFI_MEMORY_RO);
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;ap.SetMemoryAttributes=NULL;check("missing restoration API",1,1,PERMISSION_BLOCKED,0);assert(!clear_calls);
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;clear_fail=1;r=check("clear fails then restore",1,1,ATTR_CLEAR_ERROR,0);assert(r.restored_ro);
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;clear_no_effect=1;check("clear success without writable mapping",1,1,ATTR_CLEAR_ERROR,0);
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;clear_query_fail=1;check("query fails after clear",1,1,ATTR_CLEAR_ERROR,0);assert(!(pte_flags&2));
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;restore_fail=1;r=check("restore failure rolls byte back only while writable",1,1,ATTR_RESTORE_ERROR,2);assert(r.rollback&&live[RVA]==0x48&&!r.restored_ro);
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;restore_query_fail=1;r=check("restore query failure never writes protected byte",1,1,ATTR_RESTORE_ERROR,1);assert(!r.rollback&&!(pte_flags&2));
 reset();attr_present=1;attrs=EFI_MEMORY_RO;pte_flags=1;restore_lies=1;r=check("protocol claims RO but mapping remains writable",1,1,ATTR_RESTORE_ERROR,2);assert(r.rollback);
 reset();write_fail=1;check("byte readback fails",1,1,VERIFY_ERROR,1);
 reset();pte_flags=3|(1ULL<<63);check("non-executable target",1,1,PAGE_ERROR,0);
 reset();regs4=1ULL<<22;check("unsupported protection keys",1,1,PAGE_ERROR,0);
 reset();five=1;check("five-level page walk",1,1,OK,1);
 reset();leaf_level=1;check("2MiB page",1,1,OK,1);
 reset();leaf_level=2;check("1GiB page",1,1,OK,1);
 reset();pte_flags=0;check("nonpresent target page",1,1,PAGE_ERROR,0);
 printf("TOTAL %d host fault-injection cases passed\n",tests);free(live);free(fixture);return 0;
}

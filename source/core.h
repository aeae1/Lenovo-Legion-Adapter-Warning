/* GKCN65WW-only, x64 pre-OS RAM test. No NVRAM, EC or flash writes. */
#ifndef LENOVO_V4_CORE_H
#define LENOVO_V4_CORE_H
#include "uefi_min.h"
#define U16(s) ((CHAR16 *)L##s)
#define RVA 0x1030U
#define IMAGE_SIZE 0x5b40U
#define TEXT_RVA 0x2a0U
#define TEXT_SIZE 0x40baU
#define TEXT_CRC 0xed5f56ddU
#define EFI_MEMORY_RP 0x2000ULL
static const UINT8 fingerprint[16]={0x48,0x83,0xec,0x38,0xe8,0x07,0x27,0,0,0x48,0x8b,0x0d,0x20,0x42,0,0};
static EFI_GUID module_guid={0x0221d385,0x497a,0x4bbf,{0x92,0x4e,0x7b,0xdf,0xc4,0xc1,0xea,0xd8}};
static EFI_GUID trigger_guid={0x3725e246,0x7cab,0x4241,{0x95,0xa3,0xa5,0x60,0xbd,0xf4,0x4a,0x6c}};
static EFI_SYSTEM_TABLE *gST;
static EFI_BOOT_SERVICES *gBS;
static EFI_HANDLE gSelf;
static UINT8 *map;
static UINTN map_size, descriptor_size;
static void zero(void *p,UINTN n){UINT8 *b=p;while(n--)*b++=0;}
static int same(const void *a,const void *b,UINTN n){const UINT8 *x=a,*y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static UINT16 u16(const UINT8 *p){return p[0]|((UINT16)p[1]<<8);}
static UINT32 u32(const UINT8 *p){return u16(p)|((UINT32)u16(p+2)<<16);}
static UINT32 crc32(const UINT8 *p,UINTN n){UINT32 c=~0U;while(n--){c^=*p++;for(UINTN j=0;j<8;j++)c=(c>>1)^(0xedb88320U & (0U-(c&1)));}return ~c;}

enum { OK, MAP_ERROR, TARGET_MISSING, TARGET_AMBIGUOUS, IDENTITY_ERROR,
       PAGE_ERROR, ATTR_QUERY_ERROR, PERMISSION_BLOCKED, ATTR_CLEAR_ERROR,
       VERIFY_ERROR, ATTR_RESTORE_ERROR, TOO_LATE, TRIGGER_QUERY_ERROR };
static const char *reason_name(UINTN n){
 static const char *names[]={"READY_OR_PATCHED","MEMORY_MAP_ERROR","TARGET_NOT_FOUND",
 "MULTIPLE_MODULE_HANDLES","IDENTITY_MISMATCH","UNSUPPORTED_PAGE_MAPPING",
 "ATTRIBUTE_QUERY_FAILED","WRITE_PERMISSION_BLOCKED","ATTRIBUTE_CLEAR_FAILED",
 "BYTE_VERIFICATION_FAILED","ATTRIBUTE_RESTORE_FAILED","WARNING_TRIGGER_ALREADY_PRESENT",
 "TRIGGER_QUERY_FAILED"};
 return n<sizeof(names)/sizeof(names[0])?names[n]:"UNKNOWN";
}
typedef struct {UINT64 cr0,cr3,cr4,entries[5],page_size,physical;UINTN levels;
 BOOLEAN valid,rw,user,nx;} PAGE;
typedef struct {UINTN reason,modules;EFI_STATUS status,trigger_entry,trigger_before,
 attr_status,clear_status,restore_status;EFI_LOADED_IMAGE_PROTOCOL *image;
 UINT8 *target;UINT32 text_crc,mem_type;UINT64 attrs,attrs_after,attrs_restored;
 PAGE before,after;BOOLEAN attr_present,ready,patched,verified,clear_attempted,
 cleared_ro,restored_ro,rollback;EFI_MEMORY_ATTRIBUTE_PROTOCOL *attr;} RESULT;

/* These adapters are replaced ONLY by the host test translation unit. */
#ifndef HOST_TEST
static UINT64 cr0(void){UINT64 v;__asm__ volatile("mov %%cr0,%0":"=r"(v));return v;}
static UINT64 cr3(void){UINT64 v;__asm__ volatile("mov %%cr3,%0":"=r"(v));return v;}
static UINT64 cr4(void){UINT64 v;__asm__ volatile("mov %%cr4,%0":"=r"(v));return v;}
static void serialize(void){UINT32 a=0,b,c,d;__asm__ volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d)::"memory");}
static UINT64 read_entry(UINT64 table,UINTN index){return ((volatile UINT64 *)(UINTN)table)[index];}
static void store_byte(volatile UINT8 *p,UINT8 b){*p=b;serialize();}
#else
static UINT64 cr0(void);static UINT64 cr3(void);static UINT64 cr4(void);
static void serialize(void);static UINT64 read_entry(UINT64,UINTN);
static void store_byte(volatile UINT8 *,UINT8);
#endif

static EFI_STATUS take_map(void){
 UINTN size=0,key=0,ds=0;UINT32 version=0;EFI_STATUS s;
 s=gBS->GetMemoryMap(&size,NULL,&key,&ds,&version);
 if(s!=EFI_BUFFER_TOO_SMALL||ds<sizeof(EFI_MEMORY_DESCRIPTOR)||ds>1024||size>0x1000000)return EFI_DEVICE_ERROR;
 for(UINTN tries=0;tries<3;tries++){
  UINTN capacity=size+ds*32;
  s=gBS->AllocatePool(EfiBootServicesData,capacity,(void **)&map);if(EFI_ERROR(s))return s;
  size=capacity;s=gBS->GetMemoryMap(&size,(EFI_MEMORY_DESCRIPTOR *)map,&key,&ds,&version);
  if(!EFI_ERROR(s)&&ds>=sizeof(EFI_MEMORY_DESCRIPTOR)&&size<=capacity&&size%ds==0&&version==1){map_size=size;descriptor_size=ds;return EFI_SUCCESS;}
  gBS->FreePool(map);map=NULL;
  if(s!=EFI_BUFFER_TOO_SMALL||ds<sizeof(EFI_MEMORY_DESCRIPTOR)||ds>1024||size>0x1000000)return EFI_DEVICE_ERROR;
 }
 return EFI_OUT_OF_RESOURCES;
}
static EFI_MEMORY_DESCRIPTOR *descriptor(UINT64 a){
 if(!map||!descriptor_size)return NULL;
 for(UINTN i=0;i<map_size;i+=descriptor_size){
  EFI_MEMORY_DESCRIPTOR *d=(void *)(map+i);
  if(d->NumberOfPages>(~0ULL>>12))continue;
  UINT64 len=d->NumberOfPages<<12;
  if(a>=d->PhysicalStart&&a-d->PhysicalStart<len)return d;
 }return NULL;
}
/* All pre-OS x64 addresses are required to be identity mapped. Refuse MMIO,
 reserved, RP and unknown memory. Check each descriptor before any scan. */
static int ram_range(UINT64 a,UINT64 len){
 if(!len||a+len<a)return 0;
 while(len){EFI_MEMORY_DESCRIPTOR *d=descriptor(a);if(!d)return 0;
  if(d->Type<EfiLoaderCode||d->Type>EfiConventionalMemory||(d->Attribute&EFI_MEMORY_RP))return 0;
  UINT64 left=(d->NumberOfPages<<12)-(a-d->PhysicalStart),part=left<len?left:len;
  if(!part)return 0;a+=part;len-=part;
 }return 1;
}
static int fv_guid(EFI_DEVICE_PATH_PROTOCOL *path,EFI_GUID *guid){
 UINT8 *p=(UINT8 *)path;
 for(UINTN walked=0;walked<4096;){
  if(!p||!ram_range((UINTN)p,4))return 0;
  UINTN len=u16(p+2);if(len<4||len>4096-walked||!ram_range((UINTN)p,len))return 0;
  if(p[0]==4&&p[1]==6&&len==20){for(UINTN i=0;i<16;i++)((UINT8 *)guid)[i]=p[i+4];return 1;}
  if(p[0]==0x7f)return 0;
  p+=len;walked+=len;
 }return 0;
}
static int identity(EFI_LOADED_IMAGE_PROTOCOL *li,RESULT *r){
 const UINT8 *b=li->ImageBase;
 if(!b||li->ImageSize<IMAGE_SIZE||li->ImageSize>0x6000||li->ImageCodeType!=EfiBootServicesCode||!ram_range((UINTN)b,li->ImageSize))return 0;
 /* The source PE has no relocations in .text. CRC covers its ENTIRE virtual
    text section, not the mutable .data pointers or the discarded reloc area. */
 if(u16(b)!=0x5a4d||u32(b+0x3c)!=0xd0||u32(b+0xd0)!=0x4550||
    u16(b+0xd4)!=0x8664||u16(b+0xd6)!=5||u16(b+0xe4)!=0xf0||
    u16(b+0xe8)!=0x20b||u32(b+0xe8+0x38)!=IMAGE_SIZE||
    u32(b+0x1d8+8)!=TEXT_SIZE||u32(b+0x1d8+12)!=TEXT_RVA||
    !(u32(b+0x1d8+36)&0x20000000))return 0;
 r->text_crc=crc32(b+TEXT_RVA,TEXT_SIZE);
 return r->text_crc==TEXT_CRC&&same(b+RVA,fingerprint,16);
}
static UINTN find_target(RESULT *r){
 EFI_HANDLE *h=NULL;UINTN count=0;EFI_STATUS s=gBS->LocateHandleBuffer(ByProtocol,&EFI_LOADED_IMAGE_PROTOCOL_GUID,NULL,&count,&h);
 if(EFI_ERROR(s)){r->status=s;return TARGET_MISSING;}
 if(count>16384){gBS->FreePool(h);return TARGET_AMBIGUOUS;}
 for(UINTN i=0;i<count;i++){
  EFI_LOADED_IMAGE_PROTOCOL *li=NULL;EFI_GUID guid;
  if(h[i]==gSelf)continue;
  s=gBS->OpenProtocol(h[i],&EFI_LOADED_IMAGE_PROTOCOL_GUID,(void **)&li,gSelf,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if(EFI_ERROR(s)||!li)continue;
  if(fv_guid(li->FilePath,&guid)&&same(&guid,&module_guid,16)){r->modules++;r->image=li;}
 }
 gBS->FreePool(h);
 if(!r->modules)return TARGET_MISSING;
 if(r->modules!=1)return TARGET_AMBIGUOUS;
 if(!identity(r->image,r))return IDENTITY_ERROR;
 r->target=(UINT8 *)r->image->ImageBase+RVA;return OK;
}
static int walk(UINT64 va,PAGE *p){
 zero(p,sizeof(*p));p->cr0=cr0();p->cr3=cr3();p->cr4=cr4();
 int top=(p->cr4&(1ULL<<12))?4:3;
 UINTN width=top==4?57:48;UINT64 upper=va>>width;
 UINT64 expected=(va&(1ULL<<(width-1)))?((1ULL<<(64-width))-1):0;
 if(upper!=expected)return 0;
 if(!(p->cr0&(1ULL<<31))||!(p->cr4&(1ULL<<5)))return 0;
 /* Protection keys add permissions not established by this walker. */
 if(p->cr4&((1ULL<<22)|(1ULL<<24)))return 0;
 UINT64 table=p->cr3&0x000ffffffffff000ULL;p->rw=TRUE;p->user=TRUE;
 for(int level=top;level>=0;level--){
  if(!table||!ram_range(table,4096))return 0;
  UINT64 e=read_entry(table,(va>>(12+9*level))&511);p->entries[p->levels++]=e;
  if(!(e&1))return 0;
  if(!(e&2))p->rw=FALSE;if(!(e&4))p->user=FALSE;if(e&(1ULL<<63))p->nx=TRUE;
  if((e&128)&&level>2)return 0;
  if(level==0||((e&128)&&(level==1||level==2))){
   UINT64 size=1ULL<<(12+9*level),mask=size-1;
   p->page_size=size;p->physical=(e&0x000ffffffffff000ULL&~mask)|(va&mask);
   if(p->physical!=va||p->user)return 0;
   p->valid=TRUE;return 1;
  }
  table=e&0x000ffffffffff000ULL;
 }return 0;
}
static EFI_STATUS trigger_status(void){void *v=NULL;return gBS->LocateProtocol(&trigger_guid,NULL,&v);}
static UINTN inspect(RESULT *r){
 UINT64 target=(UINTN)r->target,page=target&~4095ULL;
 EFI_MEMORY_DESCRIPTOR *d=descriptor(page);
 if(!d||!ram_range(page,4096))return MAP_ERROR;
 r->mem_type=d->Type;
 /* Firmware code must actually reside in allocated code or loader/boot data,
    not unallocated conventional memory or runtime state. */
 if(d->Type!=EfiBootServicesCode&&d->Type!=EfiBootServicesData&&d->Type!=EfiLoaderCode&&d->Type!=EfiLoaderData)return MAP_ERROR;
 if(!walk(target,&r->before)||r->before.nx)return PAGE_ERROR;
 r->attr_status=gBS->LocateProtocol(&EFI_MEMORY_ATTRIBUTE_PROTOCOL_GUID,NULL,(void **)&r->attr);
 if(r->attr_status==EFI_NOT_FOUND){r->attr=NULL;return r->before.rw&&!(d->Attribute&EFI_MEMORY_RO)?OK:PERMISSION_BLOCKED;}
 if(EFI_ERROR(r->attr_status)||!r->attr||!r->attr->GetMemoryAttributes)return ATTR_QUERY_ERROR;
 r->attr_present=TRUE;
 r->attr_status=r->attr->GetMemoryAttributes(r->attr,page,4096,&r->attrs);
 if(EFI_ERROR(r->attr_status))return ATTR_QUERY_ERROR;
 if(r->attrs&(EFI_MEMORY_RP|EFI_MEMORY_XP))return PERMISSION_BLOCKED;
 if(r->attrs&EFI_MEMORY_RO){
  if(!r->attr->SetMemoryAttributes||!r->attr->ClearMemoryAttributes)return PERMISSION_BLOCKED;
  return OK; /* Candidate; the live clear/recheck is still required. */
 }
 return r->before.rw?OK:PERMISSION_BLOCKED;
}
static int restore_ro(RESULT *r,UINT64 page){
 r->restore_status=r->attr->SetMemoryAttributes(r->attr,page,4096,EFI_MEMORY_RO);serialize();
 EFI_STATUS q=r->attr->GetMemoryAttributes(r->attr,page,4096,&r->attrs_restored);
 int mapped=walk((UINTN)r->target,&r->after);
 r->restored_ro=(!EFI_ERROR(r->restore_status)&&!EFI_ERROR(q)&&r->attrs_restored==r->attrs&&mapped&&!r->after.rw&&!r->after.nx);
 return r->restored_ro;
}
static void rollback_if_writable(RESULT *r){
 PAGE now;
 /* Never use pre-change permissions to justify a later rollback store. */
 if(r->patched&&walk((UINTN)r->target,&now)&&now.rw&&!now.nx&&*(volatile UINT8 *)r->target==0xc3){
  store_byte(r->target,0x48);r->rollback=(*(volatile UINT8 *)r->target==0x48);
 }
}
static UINTN patch(RESULT *r,int require_early){
 UINT64 page=(UINTN)r->target&~4095ULL;
 r->trigger_before=trigger_status();
 if(require_early&&r->trigger_before!=EFI_NOT_FOUND)return r->trigger_before==EFI_SUCCESS?TOO_LATE:TRIGGER_QUERY_ERROR;
 if(!identity(r->image,r))return IDENTITY_ERROR;
 /* TPL_NOTIFY defers Lenovo's TPL_CALLBACK notification during the byte
    operation. Protocol/allocator methods used here permit TPL_NOTIFY. */
 EFI_TPL old=gBS->RaiseTPL(TPL_NOTIFY);UINTN result=OK;int changed=0;
 if(r->attr_present&&(r->attrs&EFI_MEMORY_RO)){
  changed=1;r->clear_attempted=TRUE;
  r->clear_status=r->attr->ClearMemoryAttributes(r->attr,page,4096,EFI_MEMORY_RO);serialize();
  EFI_STATUS q=r->attr->GetMemoryAttributes(r->attr,page,4096,&r->attrs_after);
  int mapped=walk((UINTN)r->target,&r->after);
  if(EFI_ERROR(r->clear_status)||EFI_ERROR(q)||r->attrs_after!=(r->attrs&~EFI_MEMORY_RO)||!mapped||!r->after.rw||r->after.nx){result=ATTR_CLEAR_ERROR;goto finish;}
  r->cleared_ro=TRUE;
 }else if(!walk((UINTN)r->target,&r->after)||!r->after.rw||r->after.nx){result=PERMISSION_BLOCKED;goto finish;}
 if(!same(r->target,fingerprint,16)){result=IDENTITY_ERROR;goto finish;}
 store_byte(r->target,0xc3);r->patched=TRUE;
 r->verified=(*(volatile UINT8 *)r->target==0xc3&&same(r->target+1,fingerprint+1,15));
 if(!r->verified){result=VERIFY_ERROR;rollback_if_writable(r);}
finish:
 if(changed&&!restore_ro(r,page)){
  rollback_if_writable(r);restore_ro(r,page);result=ATTR_RESTORE_ERROR;
 }
 gBS->RestoreTPL(old);return result;
}
static void run(RESULT *r,int do_patch,int early){
 zero(r,sizeof(*r));r->status=EFI_SUCCESS;r->attr_status=EFI_NOT_READY;
 r->clear_status=EFI_NOT_READY;r->restore_status=EFI_NOT_READY;r->trigger_before=EFI_NOT_READY;
 r->trigger_entry=trigger_status();
 r->status=take_map();if(EFI_ERROR(r->status)){r->reason=MAP_ERROR;return;}
 r->reason=find_target(r);if(r->reason)goto done;
 r->reason=inspect(r);if(r->reason)goto done;
 r->ready=TRUE;
 if(do_patch)r->reason=patch(r,early);
done:
 if(map){gBS->FreePool(map);map=NULL;map_size=0;descriptor_size=0;}
 if(r->reason&&!EFI_ERROR(r->status))r->status=EFI_ABORTED;
}
#endif

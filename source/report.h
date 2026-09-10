#ifndef V4_REPORT_H
#define V4_REPORT_H
#include "uefi_fs.h"
static char report[7000];static UINTN used;
static void append(const char *s){while(*s&&used<sizeof(report)-1)report[used++]=*s++;report[used]=0;}
static void hex(UINT64 v){const char *h="0123456789ABCDEF";append("0x");for(int i=15;i>=0;i--){char b[2]={h[(v>>(i*4))&15],0};append(b);}}
static void field(const char *s,UINT64 v){append(s);append("=");hex(v);append("\r\n");}
static void flag(const char *s,int b){append(s);append(b?"=1\r\n":"=0\r\n");}
static void page_report(const char *label,PAGE *p){
 append(label);append("\r\n");field("cr0",p->cr0);field("cr3",p->cr3);field("cr4",p->cr4);
 flag("page_valid",p->valid);flag("page_rw",p->rw);flag("page_user",p->user);flag("page_nx",p->nx);
 field("page_size",p->page_size);field("physical",p->physical);
 for(UINTN i=0;i<p->levels;i++)field("page_entry",p->entries[i]);
}
typedef struct {UINT16 Year;UINT8 Month,Day,Hour,Minute,Second,Pad1;UINT32 Nanosecond;UINT16 TimeZone;UINT8 Daylight,Pad2;} EFI_TIME_V4;
static void make_report(RESULT *r,const char *mode){
 used=0;append("BEGIN_LENOVO_V4_20260910\r\nmode=");append(mode);append("\r\nresult=");append(reason_name(r->reason));append("\r\n");
 EFI_TIME_V4 tm;zero(&tm,sizeof(tm));
 typedef EFI_STATUS(EFIAPI *GET_TIME)(EFI_TIME_V4 *,void *);
 EFI_STATUS ts=((GET_TIME)gST->RuntimeServices->d.GetTime)(&tm,NULL);
 field("clock_status",ts);
 if(!EFI_ERROR(ts)){field("year",tm.Year);field("month",tm.Month);field("day",tm.Day);field("hour",tm.Hour);field("minute",tm.Minute);field("second",tm.Second);}
 field("status",r->status);field("module_handles",r->modules);field("text_crc32",r->text_crc);
 field("target",(UINTN)r->target);if(r->image){field("image_base",(UINTN)r->image->ImageBase);field("image_size",r->image->ImageSize);}
 field("trigger_at_entry",r->trigger_entry);field("trigger_before_patch",r->trigger_before);
 flag("candidate_ready",r->ready);flag("patched",r->patched);flag("verified",r->verified);
 flag("clear_attempted",r->clear_attempted);flag("cleared_ro",r->cleared_ro);flag("restored_ro",r->restored_ro);flag("rollback",r->rollback);
 field("memory_type",r->mem_type);flag("attribute_protocol",r->attr_present);field("attribute_query_status",r->attr_status);
 field("attributes_before",r->attrs);field("clear_status",r->clear_status);field("attributes_after_clear",r->attrs_after);
 field("restore_status",r->restore_status);field("attributes_after_restore",r->attrs_restored);
 page_report("PAGE_BEFORE",&r->before);page_report("PAGE_AFTER",&r->after);append("END_LENOVO_V4_20260910\r\n\r\n");
}
/* Append only to the volume this program came from. Never scan internal disks
   for a log destination. A logging error has no influence on boot or patching. */
static EFI_STATUS save_report(CHAR16 *name){
 EFI_LOADED_IMAGE_PROTOCOL *li=NULL;EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs=NULL;
 EFI_FILE_PROTOCOL *root=NULL,*file=NULL;EFI_STATUS s;
 s=gBS->OpenProtocol(gSelf,&EFI_LOADED_IMAGE_PROTOCOL_GUID,(void **)&li,gSelf,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
 if(EFI_ERROR(s)||!li||!li->DeviceHandle)return EFI_NOT_FOUND;
 s=gBS->OpenProtocol(li->DeviceHandle,&EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,(void **)&fs,gSelf,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
 if(EFI_ERROR(s)||!fs||!fs->OpenVolume)return EFI_NOT_FOUND;
 s=fs->OpenVolume(fs,&root);if(EFI_ERROR(s)||!root)return s;
 s=root->Open(root,&file,name,EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,0);
 if(!EFI_ERROR(s)&&file){
  s=file->SetPosition(file,~0ULL);
  UINT64 pos=0;if(!EFI_ERROR(s))s=file->GetPosition(file,&pos);
  if(!EFI_ERROR(s)&&pos>1024*1024)s=EFI_OUT_OF_RESOURCES; /* bounded test log */
  if(!EFI_ERROR(s)){UINTN n=used;s=file->Write(file,&n,report);if(!EFI_ERROR(s)&&n!=used)s=EFI_DEVICE_ERROR;}
  if(!EFI_ERROR(s))s=file->Flush(file);file->Close(file);
 }root->Close(root);return s;
}
static void print_ascii(const char *s){CHAR16 buf[96];while(*s){UINTN n=0;while(*s&&n<95)buf[n++]=(UINT8)*s++;buf[n]=0;if(gST->ConOut)gST->ConOut->OutputString(gST->ConOut,buf);}}
static void print_field(const char *name,UINT64 value){used=0;field(name,value);print_ascii(report);}
static void wait_key(void){EFI_INPUT_KEY key;if(!gST->ConIn)return;while(gST->ConIn->ReadKeyStroke(gST->ConIn,&key)==EFI_NOT_READY)gBS->Stall(100000);}
#endif

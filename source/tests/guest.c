/* Emulator harness. Uses an inert module fixture. Never executes its entry or accesses an EC. */
#include "../core.h"
#include "../report.h"
static EFI_FILE_PROTOCOL *root;
static UINT8 *live;
static EFI_LOADED_IMAGE_PROTOCOL fake;
static EFI_HANDLE fake_handle;
static UINT8 fake_path[24];
static int failed;
typedef EFI_STATUS(EFIAPI *ALLOC_PAGES)(UINT32,UINT32,UINTN,UINT64 *);
typedef EFI_STATUS(EFIAPI *INSTALL)(EFI_HANDLE *,EFI_GUID *,UINT32,void *);
typedef EFI_STATUS(EFIAPI *LOAD)(BOOLEAN,EFI_HANDLE,void *,void *,UINTN,EFI_HANDLE *);
typedef EFI_STATUS(EFIAPI *START)(EFI_HANDLE,UINTN *,CHAR16 **);
typedef void(EFIAPI *RESET)(UINT32,EFI_STATUS,UINTN,void *);
static void expect(int ok,const char *name){print_ascii(ok?"PASS ":"FAIL ");print_ascii(name);print_ascii("\r\n");if(!ok)failed++;}
static UINTN read_file(CHAR16 *name,void *buf,UINTN capacity){EFI_FILE_PROTOCOL *f=NULL;
 if(EFI_ERROR(root->Open(root,&f,name,EFI_FILE_MODE_READ,0)))return 0;
 UINTN n=capacity;EFI_STATUS s=f->Read(f,&n,buf);f->Close(f);return EFI_ERROR(s)?0:n;
}
static void emit(RESULT *r,const char *mode){make_report(r,mode);print_ascii(report);save_report(U16("\\GUEST-RESULTS.LOG"));}
static void original(EFI_MEMORY_ATTRIBUTE_PROTOCOL *a){
 if(a)a->ClearMemoryAttributes(a,(UINTN)(live+RVA)&~4095ULL,4096,EFI_MEMORY_RO);
 store_byte(live+RVA,0x48);
}
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE self,EFI_SYSTEM_TABLE *st){
 gSelf=self;gST=st;gBS=st->BootServices;
 EFI_LOADED_IMAGE_PROTOCOL *me=NULL;EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs=NULL;
 gBS->OpenProtocol(self,&EFI_LOADED_IMAGE_PROTOCOL_GUID,(void **)&me,self,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
 if(!me)return EFI_NOT_FOUND;
 gBS->OpenProtocol(me->DeviceHandle,&EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,(void **)&fs,self,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
 if(!fs||EFI_ERROR(fs->OpenVolume(fs,&root)))return EFI_NOT_FOUND;
 UINT64 address=0;EFI_STATUS s=((ALLOC_PAGES)gBS->AllocatePages)(0,EfiBootServicesCode,6,&address);if(EFI_ERROR(s))return s;
 live=(UINT8 *)(UINTN)address;expect(read_file(U16("\\fixture.bin"),live,IMAGE_SIZE)==IMAGE_SIZE,"read original fixture");
 fake_path[0]=4;fake_path[1]=6;fake_path[2]=20;for(UINTN i=0;i<16;i++)fake_path[i+4]=((UINT8 *)&module_guid)[i];fake_path[20]=0x7f;fake_path[21]=0xff;fake_path[22]=4;
 fake.Revision=0x1000;fake.SystemTable=st;fake.ImageBase=live;fake.ImageSize=IMAGE_SIZE;
 fake.ImageCodeType=EfiBootServicesCode;fake.ImageDataType=EfiBootServicesData;fake.FilePath=(void *)fake_path;
 s=((INSTALL)gBS->InstallProtocolInterface)(&fake_handle,&EFI_LOADED_IMAGE_PROTOCOL_GUID,0,&fake);expect(s==EFI_SUCCESS,"register inert module");
 RESULT r;run(&r,0,0);emit(&r,"GUEST_DIAGNOSTIC");expect(r.reason==OK&&r.ready&&live[RVA]==0x48,"read-only identity and page walk");
 run(&r,1,1);emit(&r,"GUEST_WRITABLE");expect(r.reason==OK&&r.verified&&live[RVA]==0xc3,"writable one-byte patch");
 EFI_MEMORY_ATTRIBUTE_PROTOCOL *a=NULL;gBS->LocateProtocol(&EFI_MEMORY_ATTRIBUTE_PROTOCOL_GUID,NULL,(void **)&a);original(a);
 if(a){
  s=a->SetMemoryAttributes(a,(UINTN)(live+RVA)&~4095ULL,4096,EFI_MEMORY_RO);expect(s==EFI_SUCCESS,"set test page RO");
  run(&r,1,1);emit(&r,"GUEST_RO");expect(r.reason==OK&&r.verified&&r.cleared_ro&&r.restored_ro,"RO clear and verified restoration");original(a);
 }else{print_ascii("SKIP native RO transition: this OVMF build has no Memory Attribute Protocol.\r\n");}
 void *driver=NULL;gBS->AllocatePool(EfiBootServicesData,0x40000,&driver);
 UINTN size=read_file(U16("\\LenovoWarning-v4-Driver.efi"),driver,0x40000);EFI_HANDLE dh=NULL;
 s=((LOAD)gBS->LoadImage)(FALSE,self,NULL,driver,size,&dh);expect(s==EFI_SUCCESS,"load shipping driver as PE32+");
 if(!EFI_ERROR(s)){
  EFI_LOADED_IMAGE_PROTOCOL *dl=NULL;gBS->OpenProtocol(dh,&EFI_LOADED_IMAGE_PROTOCOL_GUID,(void **)&dl,self,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if(dl)dl->DeviceHandle=me->DeviceHandle;
  s=((START)gBS->StartImage)(dh,NULL,NULL);expect(s==EFI_SUCCESS&&live[RVA]==0xc3,"shipping driver patches and returns success");
 }
 original(a);
 EFI_HANDLE th=NULL;s=((INSTALL)gBS->InstallProtocolInterface)(&th,&trigger_guid,0,NULL);expect(s==EFI_SUCCESS,"install marker trigger");
 run(&r,1,1);emit(&r,"GUEST_TOO_LATE");expect(r.reason==TOO_LATE&&live[RVA]==0x48,"late trigger prevents write");
 live[TEXT_RVA+1]^=1;run(&r,1,0);emit(&r,"GUEST_MISMATCH");expect(r.reason==IDENTITY_ERROR&&live[RVA]==0x48,"whole-text mismatch prevents write");
 print_field("GUEST_FAILURES",failed);root->Close(root);
 ((RESET)st->RuntimeServices->d.ResetSystem)(2,EFI_SUCCESS,0,NULL);return EFI_SUCCESS;
}

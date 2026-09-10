/* VM-ONLY test application. No variable writes or Lenovo fixture is installed. */
#include "../source/core.h"
#include "../source/report.h"
static EFI_GUID global_guid = {0x8be4df61,0x93ca,0x11d2,{0xaa,0x0d,0x00,0xe0,0x98,0x03,0x2b,0x8c}};
static EFI_GUID path_guid = {0x09576e91,0x6d3f,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}};
typedef EFI_STATUS(EFIAPI *GETVAR)(CHAR16 *,EFI_GUID *,UINT32 *,UINTN *,void *);
typedef EFI_STATUS(EFIAPI *LOAD)(BOOLEAN,EFI_HANDLE,void *,void *,UINTN,EFI_HANDLE *);
typedef EFI_STATUS(EFIAPI *START)(EFI_HANDLE,UINTN *,CHAR16 **);
typedef EFI_STATUS(EFIAPI *UNLOAD)(EFI_HANDLE);
typedef void(EFIAPI *RESET)(UINT32,EFI_STATUS,UINTN,void *);
static UINT8 path[2048];
static UINTN prefix;
static int failures;
static void expect(int ok,const char *label){
 print_ascii(ok?"PASS ":"FAIL ");print_ascii(label);print_ascii("\r\n");if(!ok)failures++;
}
static void check_file(CHAR16 *name,int trusted,const char *label){
 UINTN n=0;while(name[n])n++;n=(n+1)*2;
 if(prefix+4+n+4>sizeof(path)){expect(0,"path bounds");return;}
 UINTN len=n+4;path[prefix]=4;path[prefix+1]=4;path[prefix+2]=(UINT8)len;path[prefix+3]=(UINT8)(len>>8);
 for(UINTN i=0;i<n;i++)path[prefix+4+i]=((UINT8 *)name)[i];
 path[prefix+len]=0x7f;path[prefix+len+1]=0xff;path[prefix+len+2]=4;path[prefix+len+3]=0;
 EFI_HANDLE h=NULL;EFI_STATUS s=((LOAD)gBS->LoadImage)(FALSE,gSelf,path,NULL,0,&h);
 print_field("LoadImage_status",s);
 /* Security policy may refuse loading (ACCESS_DENIED) or leave a blocked image. */
 expect(trusted?s==EFI_SUCCESS:(s==EFIERR(15)||s==EFI_SECURITY_VIOLATION),label);
 if(trusted&&s==EFI_SUCCESS){
  s=((START)gBS->StartImage)(h,NULL,NULL);expect(s==EFI_SUCCESS,"trusted driver starts and returns without Lenovo target");
 }
 if(h)((UNLOAD)gBS->UnloadImage)(h);
}
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE self,EFI_SYSTEM_TABLE *st){
 gSelf=self;gST=st;gBS=st->BootServices;
 print_ascii("BEGIN_SECURE_BOOT_VM_LOAD_CHECK\r\n");
 UINT8 value=9;UINTN size=1;EFI_STATUS s=((GETVAR)st->RuntimeServices->d.GetVariable)(U16("SecureBoot"),&global_guid,NULL,&size,&value);
 expect(s==EFI_SUCCESS&&value==1,"SecureBoot equals one in running firmware");
 value=9;size=1;s=((GETVAR)st->RuntimeServices->d.GetVariable)(U16("SetupMode"),&global_guid,NULL,&size,&value);
 expect(s==EFI_SUCCESS&&value==0,"SetupMode equals zero in running firmware");
 EFI_LOADED_IMAGE_PROTOCOL *me=NULL;EFI_DEVICE_PATH_PROTOCOL *device=NULL;
 gBS->OpenProtocol(self,&EFI_LOADED_IMAGE_PROTOCOL_GUID,(void **)&me,self,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
 if(me)gBS->OpenProtocol(me->DeviceHandle,&path_guid,(void **)&device,self,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
 expect(device!=NULL,"test disk has a device path");
 if(device){
  UINT8 *p=(UINT8 *)device;
  while(prefix+4<1024&&p[prefix]!=0x7f){UINTN n=p[prefix+2]|((UINTN)p[prefix+3]<<8);if(n<4||prefix+n>=1024)break;prefix+=n;}
  if(p[prefix]!=0x7f){expect(0,"device path bounded");}
  else{
   for(UINTN i=0;i<prefix;i++)path[i]=p[i];
   check_file(U16("\\unsigned.efi"),0,"unsigned helper rejected");
   check_file(U16("\\other.efi"),0,"untrusted signer rejected");
   check_file(U16("\\tampered.efi"),0,"signed but modified helper rejected");
   check_file(U16("\\trusted.efi"),1,"trusted signed helper accepted");
  }
 }
 print_field("SECURE_BOOT_VM_FAILURES",failures);
 ((RESET)st->RuntimeServices->d.ResetSystem)(2,EFI_SUCCESS,0,NULL);return EFI_SUCCESS;
}

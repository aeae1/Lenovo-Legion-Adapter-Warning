/* MODE=0 read-only inspection; MODE=1 manual RAM test; MODE=2 automatic driver. */
#include "core.h"
#include "report.h"
#ifndef MODE
#define MODE 0
#endif
/* Force a relocation so the PE remains relocatable even with optimization. */
void * volatile relocation_anchor=(void *)&relocation_anchor;
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE self,EFI_SYSTEM_TABLE *st){
 gSelf=self;gST=st;gBS=st->BootServices;RESULT r;
#if MODE != 2
 typedef EFI_STATUS(EFIAPI *WATCHDOG)(UINTN,UINT64,UINTN,CHAR16 *);
 ((WATCHDOG)gBS->SetWatchdogTimer)(0,0,0,NULL);
 if(st->ConOut&&st->ConOut->ClearScreen)st->ConOut->ClearScreen(st->ConOut);
#if MODE == 1
 print_ascii("Lenovo GKCN65WW V4 - MANUAL ONE-BYTE RAM TEST\r\n\r\n");
 print_ascii("Use the barrel charger. Complete Phase 1 first.\r\n");
 print_ascii("This test can change this boot's warning callback and its page permissions.\r\n");
 print_ascii("Press uppercase P to run. Any other key exits without patching.\r\n");
 EFI_INPUT_KEY key={0};if(!st->ConIn)return EFI_SUCCESS;
 while(st->ConIn->ReadKeyStroke(st->ConIn,&key)==EFI_NOT_READY)gBS->Stall(100000);
 if(key.UnicodeChar!='P')return EFI_SUCCESS;
#else
 print_ascii("Lenovo GKCN65WW V4 - READ-ONLY DIAGNOSTIC\r\n");
 print_ascii("No code, NVRAM, or memory-permission changes. USB log only.\r\n\r\n");
#endif
#endif
 run(&r,MODE!=0,MODE==2);
 make_report(&r,MODE==0?"DIAGNOSTIC":MODE==1?"MANUAL":"AUTOMATIC_DRIVER");
 EFI_STATUS log_status=save_report(MODE==0?U16("\\LENOVO_V4_DIAGNOSTIC.LOG"):MODE==1?U16("\\LENOVO_V4_MANUAL.LOG"):U16("\\LENOVO_V4_DRIVER.LOG"));
#if MODE != 2
 print_ascii("RESULT: ");print_ascii(reason_name(r.reason));print_ascii("\r\n");
 print_field("module_handles",r.modules);print_field("text_crc32",r.text_crc);
 print_field("target",(UINTN)r.target);print_field("memory_type",r.mem_type);
 print_field("page_mapping_valid",r.before.valid);print_field("page_writable",r.before.rw);
 print_field("cr0",r.before.cr0);print_field("attribute_protocol",r.attr_present);
 print_field("attribute_query_status",r.attr_status);print_field("attributes_before",r.attrs);
 print_field("patched",r.patched);print_field("verified",r.verified);print_field("restored_ro",r.restored_ro);
 print_field("log_write_status",log_status);
 if(MODE==0)print_ascii("Send the diagnostic log before Phase 2.\r\n");
 if(MODE==1)print_ascii("Power off fully after this test.\r\n");
 print_ascii("Press any key to exit.\r\n");wait_key();
#else
 (void)log_status;
#endif
 /* The automatic helper never creates a firmware error prompt itself. */
 return EFI_SUCCESS;
}

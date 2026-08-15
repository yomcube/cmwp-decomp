#ifndef PRIVATE_ESP_H
#define PRIVATE_ESP_H

#include <private/es/estypes.h>

/**
 * https://wiibrew.org/wiki//dev/es
 * https://github.com/devkitPro/libogc/blob/master/gc/ogc/es.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/es.h
 */

ESError ESP_InitLib();
ESError ESP_CloseLib();

ESError ESP_ImportTicket(const ESTicket* ticket, void* certs, u32 nCerts, void* crls, u32 nCrls, BOOL unk);

ESError ESP_ImportTitleInit(ESTitleMeta* tmd, u32 tmdSize, void* certs, u32 nCerts, void* crls, u32 nCrls, int unknown0, int unknown1);
ESFd ESP_ImportContentBegin(ESTitleId titleId, ESContentId contentId);
ESError ESP_ImportContentData(ESFd fd, void* buf, u32 bufSize);
ESError ESP_ImportContentEnd(ESFd fd);
ESError ESP_ImportTitleDone();
ESError ESP_ImportTitleCancel();

ESError ESP_LaunchTitle(ESTitleId titleId, ESTicketView* ticket);

ESFd ESP_OpenContentFile(ESContentId contentId);
ESFd ESP_OpenTitleContentFile(ESTitleId titleId, ESTicketView* ticketView, ESContentId contentId);
ESError ESP_ReadContentFile(ESFd fd, void* buf, u32 bufSize);
ESError ESP_SeekContentFile(ESFd fd, s32 offset, u32 whence);
ESError ESP_CloseContentFile(ESFd fd);

ESError ESP_ListOwnedTitles(ESTitleId* titleIds, u32* numTitles);

ESError ESP_ListTitlesOnCard(ESTitleId* titleIds, u32* numTitles);
ESError ESP_ListTitleContentsOnCard(ESTitleId titleId, ESContentId* contentIds, u32* numContents);
ESError ESP_ListTmdContentsOnCard(ESTitleMeta* tmd, u32 tmdSize, ESContentId* contentIds, u32* numContents);

ESError ESP_GetTicketViews(ESTitleId titleId, ESTicketView* ticketViews, u32* numTicketViews);
ESError ESP_DiGetTicketView(void* ticket, ESTicketView* ticketView);

ESError ESP_DiGetTmd(ESTitleMeta* tmd, u32* tmdSize);
ESError ESP_GetTmd(ESTitleId titleId, ESTitleMeta* tmd, u32* tmdSize);
ESError ESP_GetTmdSize(ESTitleMeta* tmd, u32* tmdSize);
ESError ESP_GetTmdView(ESTitleId titleId, ESTmdView* tmdView, u32* tmdSize);

ESError ESP_GetDataDir(ESTitleId titleId, char* dataDir);
ESError ESP_GetTitleId(ESTitleId* titleId);
ESError ESP_GetDeviceId(ESDeviceId* deviceId);
ESError ESP_GetConsumption(ESTicketId ticketId, ESLpEntry* limitEntries, u32* numEntries);
ESError ESP_GetDeviceCert(IOSCSigRsa2048* deviceCert);

ESError ESP_Encrypt(u32 keyNum, u8* iv, u8* input, u32 size, u8* output);
ESError ESP_Decrypt(u32 keyNum, u8* iv, u8* input, u32 size, u8* output);

ESError ESP_Sign(void* data, u32 dataSize, IOSCSigDummy sig, IOSCSigRsa2048* sigSize);
ESError ESP_VerifySign(void* data, u32 dataSize, IOSCSigDummy sig, void* certs, u32 nCerts);

ESError ESP_DeleteTitleContent(ESTitleId titleId);
ESError ESP_DeleteContent(ESTitleId titleId, ESContentId contentId);

ESError ESP_ExportTitleInit(ESTitleId titleId, ESDeviceId deviceId, ESTicketId ticketId, void* certs, u32 nCerts, void* crls, u32 nCrls, int unknown,
                            void* ticket, void* tmd, u32 tmdSize);
ESError ESP_ExportContentBegin(ESTitleId titleId, ESContentId contentId);
ESError ESP_ExportContentData(ESFd fd, void* data, u32 dataSize);
ESError ESP_ExportContentEnd(ESFd fd);
ESError ESP_ExportTitleDone();

ESError ESP_GetTicket(ESTicketView* ticketView, ESTicket* ticket);

#endif  // PRIVATE_ESP_H

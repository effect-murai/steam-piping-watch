/*
 * Protection.h
 *
 *  Created on: 2020/11/05
 *      Author: k.tasaki
 */

#ifndef PROTECTION_H_
#define PROTECTION_H_

#include <windows.h>
#include "guid.h"

extern bool registInstallKey(LPCTSTR licenseKey);
extern void deleteInstallKey();
extern bool checkInstallKey();
extern bool checkLicense();
extern std::wstring getInstallKeyInfo();
extern std::wstring getMachineGuid();
extern std::wstring getVersion();
extern bool isAdministrator();

#endif /* PROTECTION_H_ */

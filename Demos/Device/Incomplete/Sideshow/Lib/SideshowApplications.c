/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "SideshowApplications.h"

SideShow_Application_t InstalledApplications[MAX_APPLICATIONS];


SideShow_Application_t* SideShow_GetFreeApplication(void)
{
	for (uint8_t App = 0; App < ARRAY_ELEMENTS(InstalledApplications); App++)
	{
		if (!(InstalledApplications[App].InUse))
		  return &InstalledApplications[App];
	}

	return NULL;
}

SideShow_Application_t* SideShow_GetApplicationFromGUID(GUID_t* const GUID)
{
	for (uint8_t App = 0; App < ARRAY_ELEMENTS(InstalledApplications); App++)
	{
		if (InstalledApplications[App].InUse)
		{
			if (GUID_COMPARE(&InstalledApplications[App].ApplicationID, GUID))
			  return &InstalledApplications[App];
		}
	}

	return NULL;
}


/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#define  INCLUDE_FROM_SIDESHOWCOMMANDS_H
#include "SideshowCommands.h"

UNICODE_STRING_t(80) UserSID       = {LengthInBytes: sizeof(SECURITY_INTERACTIVE_RID_SID),
                                      UnicodeString: SECURITY_INTERACTIVE_RID_SID};

Unicode_String_t DeviceName        = {LengthInBytes: sizeof(L"LUFA Sideshow Device"),
                                      UnicodeString: L"LUFA Sideshow Device"};

Unicode_String_t Manufacturer      = {LengthInBytes: sizeof(L"Dean Camera"),
                                      UnicodeString: L"Dean Camera"};

Unicode_String_t SupportedLanguage = {LengthInBytes: sizeof(L"en-US:1"),
                                      UnicodeString: L"en-US:1"};

void Sideshow_ProcessCommandPacket(void)
{
	SideShow_PacketHeader_t PacketHeader;

	Endpoint_SelectEndpoint(SIDESHOW_OUT_EPNUM);
	Endpoint_Read_Stream_LE(&PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);

	PacketHeader.Type.TypeFields.Response = true;

	printf("\r\nCmd: %lX", (PacketHeader.Type.TypeLong & 0x00FFFFFF));

	switch (PacketHeader.Type.TypeLong & 0x00FFFFFF)
	{
		case SIDESHOW_CMD_PING:
			SideShow_Ping(&PacketHeader);
			break;
		case SIDESHOW_CMD_SYNC:
			SideShow_Sync(&PacketHeader);
			break;
		case SIDESHOW_CMD_GET_CURRENT_USER:
			SideShow_GetCurrentUser(&PacketHeader);
			break;
		case SIDESHOW_CMD_SET_CURRENT_USER:
			SideShow_SetCurrentUser(&PacketHeader);
			break;
		case SIDESHOW_CMD_GET_CAPABILITIES:
			SideShow_GetCapabilities(&PacketHeader);
			break;
		case SIDESHOW_CMD_GET_DEVICE_NAME:
			SideShow_GetString(&PacketHeader, &DeviceName);
			break;
		case SIDESHOW_CMD_GET_MANUFACTURER:
			SideShow_GetString(&PacketHeader, &Manufacturer);
			break;
		case SIDESHOW_CMD_GET_APPLICATION_ORDER:
			SideShow_GetApplicationOrder(&PacketHeader);
			break;
		case SIDESHOW_CMD_GET_SUPPORTED_ENDPOINTS:
			SideShow_GetSupportedEndpoints(&PacketHeader);
			break;
		case SIDESHOW_CMD_ADD_APPLICATION:
			SideShow_AddApplication(&PacketHeader);
			break;
		case SIDESHOW_CMD_ADD_CONTENT:
			SideShow_AddContent(&PacketHeader);
			break;
		case SIDESHOW_CMD_DELETE_CONTENT:
			SideShow_DeleteContent(&PacketHeader);
			break;
		case SIDESHOW_CMD_DELETE_ALL_CONTENT:
			SideShow_DeleteAllContent(&PacketHeader);
			break;
		case SIDESHOW_CMD_DELETE_APPLICATION:
			SideShow_DeleteApplication(&PacketHeader);
			break;
		case SIDESHOW_CMD_DELETE_ALL_APPLICATIONS:
			SideShow_DeleteAllApplications(&PacketHeader);
			break;
		default:
			PacketHeader.Length -= sizeof(SideShow_PacketHeader_t);

			Endpoint_Discard_Stream(PacketHeader.Length, NULL);
			Endpoint_ClearOUT();

			PacketHeader.Length   = sizeof(SideShow_PacketHeader_t);
			PacketHeader.Type.TypeFields.NAK = true;

			Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
			Endpoint_Write_Stream_LE(&PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
			Endpoint_ClearIN();

			printf(" UNK");
	}
}

static void SideShow_Ping(SideShow_PacketHeader_t* const PacketHeader)
{
	Endpoint_ClearOUT();

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_Sync(SideShow_PacketHeader_t* const PacketHeader)
{
	GUID_t ProtocolGUID;

	Endpoint_Read_Stream_LE(&ProtocolGUID, sizeof(GUID_t), NULL);
	Endpoint_ClearOUT();

	if (!(GUID_COMPARE(&ProtocolGUID, (uint32_t[])STANDARD_PROTOCOL_GUID)))
	  PacketHeader->Type.TypeFields.NAK = true;

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_Write_Stream_LE(&ProtocolGUID, sizeof(GUID_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_GetCurrentUser(SideShow_PacketHeader_t* const PacketHeader)
{
	Endpoint_ClearOUT();

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t) + sizeof(uint32_t) + UserSID.LengthInBytes;

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	SideShow_Write_Unicode_String(&UserSID);
	Endpoint_ClearIN();
}

static void SideShow_SetCurrentUser(SideShow_PacketHeader_t* const PacketHeader)
{
	SideShow_Read_Unicode_String(&UserSID, sizeof(UserSID.UnicodeString));
	Endpoint_ClearOUT();

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_GetCapabilities(SideShow_PacketHeader_t* const PacketHeader)
{
	SideShow_PropertyKey_t  Property;
	SideShow_PropertyData_t PropertyData;

	Endpoint_Read_Stream_LE(&Property, sizeof(SideShow_PropertyKey_t), NULL);
	Endpoint_ClearOUT();

	printf(" ID: %lu", Property.PropertyID);

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	if (GUID_COMPARE(&Property.PropertyGUID, (uint32_t[])SIDESHOW_PROPERTY_GUID))
	{
		switch (Property.PropertyID)
		{
			case PROPERTY_SIDESHOW_SCREENTYPE:
				PropertyData.DataType    = VT_I4;
				PropertyData.Data.Data32 = ScreenText;
				PacketHeader->Length += sizeof(uint32_t);

				break;
			case PROPERTY_SIDESHOW_SCREENWIDTH:
			case PROPERTY_SIDESHOW_CLIENTWIDTH:
				PropertyData.DataType    = VT_UI2;
				PropertyData.Data.Data16 = 16;
				PacketHeader->Length += sizeof(uint16_t);

				break;
			case PROPERTY_SIDESHOW_SCREENHEIGHT:
			case PROPERTY_SIDESHOW_CLIENTHEIGHT:
				PropertyData.DataType    = VT_UI2;
				PropertyData.Data.Data16 = 2;
				PacketHeader->Length += sizeof(uint16_t);

				break;
			case PROPERTY_SIDESHOW_COLORDEPTH:
				PropertyData.DataType    = VT_UI2;
				PropertyData.Data.Data16 = 1;
				PacketHeader->Length += sizeof(uint16_t);

				break;
			case PROPERTY_SIDESHOW_COLORTYPE:
				PropertyData.DataType    = VT_UI2;
				PropertyData.Data.Data16 = BlackAndWhiteDisplay;
				PacketHeader->Length += sizeof(uint16_t);

				break;
			case PROPERTY_SIDESHOW_DATACACHE:
				PropertyData.DataType    = VT_BOOL;
				PropertyData.Data.Data16 = false;
				PacketHeader->Length += sizeof(uint16_t);

				break;
			case PROPERTY_SIDESHOW_SUPPORTEDLANGS:
			case PROPERTY_SIDESHOW_CURRENTLANG:
				PropertyData.DataType    = VT_LPWSTR;
				PropertyData.Data.DataPointer = &SupportedLanguage;
				PacketHeader->Length += SupportedLanguage.LengthInBytes;

				break;
			default:
				PropertyData.DataType    = VT_EMPTY;
				break;
		}
	}
	else if (GUID_COMPARE(&Property.PropertyGUID, (uint32_t[])DEVICE_PROPERTY_GUID))
	{
		switch (Property.PropertyID)
		{
			case PROPERTY_DEVICE_DEVICETYPE:
				PropertyData.DataType    = VT_UI4;
				PropertyData.Data.Data32 = GenericDevice;
				PacketHeader->Length += sizeof(uint32_t);

				break;
		}
	}
	else
	{
		PacketHeader->Type.TypeFields.NAK = true;

		printf(" WRONG GUID");
		printf(" %lX %lX %lX %lX", Property.PropertyGUID.Chunks[0], Property.PropertyGUID.Chunks[1],
		                           Property.PropertyGUID.Chunks[2],  Property.PropertyGUID.Chunks[3]);
	}

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);

	if (!(PacketHeader->Type.TypeFields.NAK))
	{
		switch (PropertyData.DataType)
		{
			case VT_UI4:
			case VT_I4:
				Endpoint_Write_Stream_LE(&PropertyData.Data.Data32, sizeof(uint32_t), NULL);
				break;
			case VT_UI2:
			case VT_I2:
			case VT_BOOL:
				Endpoint_Write_Stream_LE(&PropertyData.Data.Data16, sizeof(uint16_t), NULL);
				break;
			case VT_LPWSTR:
				SideShow_Write_Unicode_String((Unicode_String_t*)PropertyData.Data.Data16);
				break;
		}
	}

	Endpoint_ClearIN();
	return;
}

static void SideShow_GetString(SideShow_PacketHeader_t* const PacketHeader,
                               void* const UnicodeStruct)
{
	Endpoint_ClearOUT();

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t) +
	                       sizeof(uint32_t) + ((Unicode_String_t*)UnicodeStruct)->LengthInBytes;

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	SideShow_Write_Unicode_String(UnicodeStruct);
	Endpoint_ClearIN();
}

static void SideShow_GetApplicationOrder(SideShow_PacketHeader_t* const PacketHeader)
{
	uint8_t  TotalApplications = 0;

	Endpoint_ClearOUT();

	for (uint8_t App = 0; App < MAX_APPLICATIONS; App++)
	{
		if (InstalledApplications[App].InUse)
		  TotalApplications++;
	}

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t) +
	                       sizeof(uint32_t) + (TotalApplications * sizeof(GUID_t));

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_Write_32_LE(TotalApplications);

	for (uint8_t App = 0; App < MAX_APPLICATIONS; App++)
	{
		if (InstalledApplications[App].InUse)
		  Endpoint_Write_Stream_LE(&InstalledApplications[App].ApplicationID, sizeof(GUID_t), NULL);
	}

	Endpoint_ClearIN();
}

static void SideShow_GetSupportedEndpoints(SideShow_PacketHeader_t* const PacketHeader)
{
	GUID_t SupportedEndpointGUID = (GUID_t){Chunks: SIMPLE_CONTENT_FORMAT_GUID};

	Endpoint_ClearOUT();

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t) + sizeof(uint32_t) + sizeof(GUID_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_Write_32_LE(1);
	Endpoint_Write_Stream_LE(&SupportedEndpointGUID, sizeof(GUID_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_AddApplication(SideShow_PacketHeader_t* const PacketHeader)
{
	SideShow_Application_t* CurrApp;
	GUID_t                  ApplicationID;

	Endpoint_Read_Stream_LE(&ApplicationID, sizeof(GUID_t), NULL);

	CurrApp = SideShow_GetApplicationFromGUID(&ApplicationID);

	if (CurrApp == NULL)
	  CurrApp = SideShow_GetFreeApplication();

	if (CurrApp == NULL)
	{
		PacketHeader->Length -= sizeof(SideShow_PacketHeader_t) + sizeof(GUID_t);

		Endpoint_Discard_Stream(PacketHeader->Length, NULL);
		Endpoint_ClearOUT();

		PacketHeader->Type.TypeFields.NAK = true;
	}
	else
	{
		CurrApp->ApplicationID = ApplicationID;
		Endpoint_Read_Stream_LE(&CurrApp->EndpointID, sizeof(GUID_t), NULL);
		SideShow_Read_Unicode_String(&CurrApp->ApplicationName, sizeof(CurrApp->ApplicationName.UnicodeString));
		Endpoint_Read_Stream_LE(&CurrApp->CachePolicy, sizeof(uint32_t), NULL);
		Endpoint_Read_Stream_LE(&CurrApp->OnlineOnly, sizeof(uint32_t), NULL);
		SideShow_Discard_Byte_Stream();
		SideShow_Discard_Byte_Stream();
		SideShow_Discard_Byte_Stream();
		Endpoint_ClearOUT();

		CurrApp->InUse = true;
		CurrApp->HaveContent = false;
		CurrApp->CurrentContentID = 1;
	}

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_DeleteApplication(SideShow_PacketHeader_t* const PacketHeader)
{
	GUID_t ApplicationGUID;

	Endpoint_Read_Stream_LE(&ApplicationGUID, sizeof(GUID_t), NULL);
	Endpoint_ClearOUT();

	SideShow_Application_t* AppToDelete = SideShow_GetApplicationFromGUID(&ApplicationGUID);

	if (AppToDelete != NULL)
	  AppToDelete->InUse = false;
	else
	  PacketHeader->Type.TypeFields.NAK = true;

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_DeleteAllApplications(SideShow_PacketHeader_t* const PacketHeader)
{
	Endpoint_ClearOUT();

	for (uint8_t App = 0; App < MAX_APPLICATIONS; App++)
	  InstalledApplications[App].InUse = false;

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_AddContent(SideShow_PacketHeader_t* const PacketHeader)
{
	GUID_t ApplicationID;
	GUID_t EndpointID;
	SideShow_Application_t* Application;

	Endpoint_Read_Stream_LE(&ApplicationID, sizeof(GUID_t), NULL);
	Endpoint_Read_Stream_LE(&EndpointID, sizeof(GUID_t), NULL);

	Application = SideShow_GetApplicationFromGUID(&ApplicationID);

	if (Application == NULL)
	{
		SideShow_Discard_Byte_Stream();
		PacketHeader->Type.TypeFields.NAK = true;
	}
	else if (!(SideShow_AddSimpleContent(PacketHeader, Application)))
	{
		PacketHeader->Type.TypeFields.NAK = true;
	}

	Endpoint_ClearOUT();

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_DeleteContent(SideShow_PacketHeader_t* const PacketHeader)
{
	GUID_t   ApplicationID;
	GUID_t   EndpointID;
	uint32_t ContentID;

	Endpoint_Read_Stream_LE(&ApplicationID, sizeof(GUID_t), NULL);
	Endpoint_Read_Stream_LE(&EndpointID, sizeof(GUID_t), NULL);
	Endpoint_Read_Stream_LE(&ContentID, sizeof(uint32_t), NULL);
	Endpoint_ClearOUT();

	SideShow_Application_t* Application = SideShow_GetApplicationFromGUID(&ApplicationID);

	if ((Application != NULL) && (Application->CurrentContentID == ContentID))
	  Application->HaveContent = false;
	else
	  PacketHeader->Type.TypeFields.NAK = true;

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}

static void SideShow_DeleteAllContent(SideShow_PacketHeader_t* const PacketHeader)
{
	GUID_t ApplicationID;
	GUID_t EndpointID;

	Endpoint_Read_Stream_LE(&ApplicationID, sizeof(GUID_t), NULL);
	Endpoint_Read_Stream_LE(&EndpointID, sizeof(GUID_t), NULL);
	Endpoint_ClearOUT();

	SideShow_Application_t* Application = SideShow_GetApplicationFromGUID(&ApplicationID);

	if (Application != NULL)
	  Application->HaveContent = false;
	else
	  PacketHeader->Type.TypeFields.NAK = true;

	PacketHeader->Length = sizeof(SideShow_PacketHeader_t);

	Endpoint_SelectEndpoint(SIDESHOW_IN_EPNUM);
	Endpoint_Write_Stream_LE(PacketHeader, sizeof(SideShow_PacketHeader_t), NULL);
	Endpoint_ClearIN();
}


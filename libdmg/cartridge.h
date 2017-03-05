#ifndef _CARTRIDGE_H_
#define _CARTRIDGE_H_

#include "environment.h"

namespace libdmg
{
	class Cartridge
	{
	public:
		const uint16_t HEADER_OFFSET = 0x0100;

		struct Header
		{
			uint8_t entryPoint[4];
			uint8_t logo[48];

			union
			{
				struct
				{
					char title[16];
				} v1;

				struct
				{
					char title[15];
					uint8_t gbcFlag;
				} v2;

				struct
				{
					char title[11];
					char manufacturer[4];
					uint8_t gbcFlag;
				} v3;
			};

			char licensee[2];
			uint8_t sgbFlag;
			uint8_t cartridgeHardware;
			uint8_t romSize;
			uint8_t ramSize;
			uint8_t destinationCode;
			uint8_t legacyLicensee;
			uint8_t romVersion;
			uint8_t headerChecksum;
			uint16_t romChecksum;
		};
		
		const Header* header;

		const uint8_t* rom;

	public:
		Cartridge(const uint8_t* buffer);

	};

}

#endif
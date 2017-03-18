#ifndef _COLOR_H_
#define _COLOR_H_


namespace WinBoy
{
	struct Color
	{
		union
		{
			float data[4];

			struct
			{
				float r, g, b, a;
			};
		};

		float& operator[](const int index)
		{
			return data[index];
		}

		const float& operator[](const int index) const
		{
			return data[index];
		}


	};
}

#endif
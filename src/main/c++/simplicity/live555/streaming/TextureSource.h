#ifndef TEXTURESOURCE_H_
#define TEXTURESOURCE_H_

#include <simplicity/rendering/Texture.h>

#include "Source.h"

namespace simplicity
{
	namespace live555
	{
		class TextureSource : public Source
		{
			public:
				TextureSource(const Texture& texture);

				~TextureSource();

				bool getNextFrame(SourceFrame& sourceFrame);

			private:
				// FFMPEG pollutes the global namespace (being C) and conflicts with simplicity symbols.
				// To workaround this I have used a void pointer here which I populate with a type defined in the
				// implementation file. This allows me to move the FFMPEG headers to the implementation file.
				void* codec;

				const Texture* texture;

				void initConversion();
		};
	}
}

#endif /* TEXTURESOURCE_H_ */

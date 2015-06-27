#ifndef SOURCEFRAME_H_
#define SOURCEFRAME_H_

namespace simplicity
{
	namespace live555
	{
		struct SourceFrame
		{
			unsigned char* frameData; // in

			unsigned int maxFrameSize; // in

			unsigned int frameSize; // out

			unsigned int numTruncatedBytes; // out

			struct timeval presentationTime; // out

			unsigned int durationInMicroseconds;
		};
	}
}

#endif /* SOURCEFRAME_H_ */

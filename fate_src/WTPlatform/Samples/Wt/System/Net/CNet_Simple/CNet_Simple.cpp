// CNet_Simple.cpp: a trivial program to download data from a url
// and print it to the console.

#include <stdio.h>
#include <windows.h> // need windows if we're using Net.h
#include "WT/System/Net/Net.h" // an easy-to-use include file

int main()
{
	using namespace WT::System::Net;
	const char* const Url = "http://www.google.com/index.html";
	const int KLen = 64*1024;

	printf ("WTPlatform WT::System::Net simple download example\n");

	//
	// Here's the code that does an HTTP request.
	//
	CHttpWebRequest* pRequest = CWebRequest::CreateHttp (Url);
	CWebResponse& Response = pRequest->GetResponse();
	WT::System::IO::CStream* Outstream = Response.GetResponseStream();

	//
	// The rest of the code simply moves the data from the
	// 'Stream' out to the console
	//
	WT::uint8* Buffer = new WT::uint8 [KLen+1];
	for (WT::int32 Len=Outstream->Read (Buffer, 0, KLen); Len; Len=Outstream->Read (Buffer, 0, KLen))
	{
		Buffer [Len] = '\0'; // Buffer is made big enough for a NUL char at the end
		puts ((const char*)Buffer);
	}

	//
	// Finally delete the objects that we created
	//    The Outstream is not deleted because it's
	//    embedded in the Response.
	//
	delete[] Buffer;
	delete pRequest;

	return 0;
}
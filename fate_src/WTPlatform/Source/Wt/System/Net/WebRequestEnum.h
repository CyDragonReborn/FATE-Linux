/*
	WebRequestEnum.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#pragma once

namespace WT
{
namespace System
{
namespace Net
{
namespace DownloadStatus
{
	// Enumerated values for the current status of a download.
	// These let us track the current progress of a download
	// in a simplified fashion.
	enum Enum {
		KNotStarted,
		KDownloading,
		KPaused,
		KDoneOk,
		KDoneFail, // e.g., because it was aborted
	};
}

namespace HttpStatusCode
{
	/// Enumerated values for each of the possible Http error codes plus an additional "NotSet"
	/// code for when a real Http error code is not valid.  Each of the values has the same
	/// numerical value as the Http error code -- for example, NotFound is 404.
	enum Enum { NotSet=-1, 
		Continue=100,
		SwitchingProtocols=101,

		OK=200,
		Created=201,
		Accepted=202, 
		NonAuthoritativeInformation=203,
		NoContent=204,
		ResetContent=205,
		PartialContent=206,

		Ambiguous=300, 
		MultipleChoices=300,
		Moved=301, 
		MovedPermanently=301,
		Found=302,
		Redirect=302,
		RedirectMethod=303,
		SeeOther=303, 
		NotModified=304,
		UseProxy=305,
		Unused=306,
		RedirectKeepVerb=307,
		TemporaryRedirect=307,

		BadRequest=400, 
		Unauthorized=401,
		PaymentRequired=402,
		Forbidden=403,
		NotFound=404,
		MethodNotAllowed=405,
		NotAcceptable=406,
		ProxyAuthenticationRequired=407,
		RequestTimeout=408,
		Conflict=409, 
		Gone=410,
		LengthRequired=411,
		PreconditionFailed=412,
		RequestEntryTooLarge=413,
		RequestUriTooLong=414,
		UnsupportedMediaType=415,
		RequestedRangeNotSatisfiable=416,
		ExpectationFailed=417,

		InternalServerError=500,
		NotImplemented=501,
		BadGateway=502, 
		ServiceUnavailable=503,
		GatewayTimeout=504,
		HttpVersionNotSupported=505,

	};

} // end namespace HttpStatusCode

} // end namespace Net
} // end namespace System
} // end namespace WT
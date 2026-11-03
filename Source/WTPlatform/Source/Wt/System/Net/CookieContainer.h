/*
	CookieContainer.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#pragma once

#include <map>
#include "WT/System/Net/Cookie.h"
#include "WT/System/Net/CookieCollection.h"

namespace WT
{
namespace System
{
namespace Net
{

typedef std::map<string, CCookieCollection* > CCookieMap;

typedef SmartPointer<class CCookieContainer> HCookieContainer;
class CCookieContainer  : public CObject 
{
public:
	WT_CLASS( CCookieContainer, CObject );

	CCookieContainer();
	virtual ~CCookieContainer();
	//// pCookie can be null, in which case a new cookie collection is
	// created but not added to.
	void Add (const WT::System::CUri& Uri, WT::System::Net::CCookie* pCookie);

	/// Dumps the list of contains to stdout.  Debugging only.
	void Dump() const;

	WT::System::Net::CCookieCollection* GetCookies(const WT::System::CUri& Uri);

	/// WT specific routine to initialize a collection from a standard
	/// Cookie: string from the client.  Buffer can be null, in which
	/// case the underlying collection is made but not filled in.
	void WTInit (const WT::System::CUri& Uri, int8* const Buffer);


private:
	CCookieMap m_Data;

};

} // end namespace Net
} // end namespace System
} // end namespace WT


#ifdef NET_EXAMPLE

//
// Following is an example of using CookieContainers from HttpWebRequests;
// it's from the HttpWebRequest.CookieContainer documentation (July 2005)
//

using System.Net;
using System;
namespace Examples.System.Net.Cookies
{
    // This example is run at the command line.
    // Specify one argument: the name of the host to 
    // send the request to.
    // If the request is sucessful, the example displays the contents of the cookies
    // returned by the host.
    
    public class CookieExample
    {   
        public static void Main(string[] args)
        {   
            if (args == null || args.Length != 1)
            {
                Console.WriteLine("Specify the URL to receive the request.");
                Environment.Exit(1);
            }
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(args[0]);
            request.CookieContainer = new CookieContainer();
        
            HttpWebResponse response = (HttpWebResponse) request.GetResponse();
            response.Cookies = request.CookieContainer.GetCookies(request.RequestUri);
            

            // Print the properties of each cookie.
            foreach (Cookie cook in response.Cookies)
            {
                Console.WriteLine("Cookie:");
                Console.WriteLine("{0} = {1}", cook.Name, cook.Value);
                Console.WriteLine("Domain: {0}", cook.Domain);
                Console.WriteLine("Path: {0}", cook.Path);
                Console.WriteLine("Port: {0}", cook.Port);
                Console.WriteLine("Secure: {0}", cook.Secure);
             
                Console.WriteLine("When issued: {0}", cook.TimeStamp);
                Console.WriteLine("Expires: {0} (expired? {1})", 
                    cook.Expires, cook.Expired);
                Console.WriteLine("Don't save: {0}", cook.Discard);    
                Console.WriteLine("Comment: {0}", cook.Comment);
                Console.WriteLine("Uri for comments: {0}", cook.CommentUri);
                Console.WriteLine("Version: RFC {0}" , cook.Version == 1 ? "2109" : "2965");

                // Show the string representation of the cookie.
                Console.WriteLine ("String: {0}", cook.ToString());
            }
        }
    }
}
#endif

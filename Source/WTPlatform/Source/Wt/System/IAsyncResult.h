// File: IAsyncResult.h
// Contains a definition of the IAsyncResult interface

#pragma once
namespace WT
{
namespace System
{

class IAsyncResult
{
public:
	virtual ~IAsyncResult() { }
	virtual void* GetAsyncState() const = 0;
	virtual bool GetIsCompleted() const = 0;
};

} // end namespace System
} // end namespace WT
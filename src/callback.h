/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/



#include <stdint.h> // uint64_t

#ifndef CALLBACK_H
#define CALLBACK_H

namespace SST { namespace MacSim {

template <typename ReturnT, typename Param1T>
class CallbackBase1
{
public:
	virtual ~CallbackBase1() = 0;
	virtual ReturnT operator()(Param1T) = 0;
};

template <typename ReturnT, typename Param1T> MacSim::CallbackBase1<ReturnT,Param1T>::~CallbackBase1() {}

template <typename ConsumerT, typename ReturnT, typename Param1T> 
class Callback1: public CallbackBase1<ReturnT,Param1T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T);

public:
	Callback1( ConsumerT* const object, PtrMember member) : object(object), member(member) { } 
	Callback1( const Callback1<ConsumerT,ReturnT,Param1T>& e ) : object(e.object), member(e.member) { } 
	ReturnT operator()(Param1T param1) { return (const_cast<ConsumerT*>(object)->*member) (param1); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};



template <typename ReturnT, typename Param1T, typename Param2T>
class CallbackBase2
{
public:
	virtual ~CallbackBase2() = 0;
	virtual ReturnT operator()(Param1T,Param2T) = 0;
};

template <typename ReturnT, typename Param1T, typename Param2T> MacSim::CallbackBase2<ReturnT,Param1T,Param2T>::~CallbackBase2() {}

template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T> 
class Callback2: public CallbackBase2<ReturnT,Param1T,Param2T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T);

public:
	Callback2( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback2( const Callback2<ConsumerT,ReturnT,Param1T,Param2T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};



template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
class CallbackBase3
{
public:
	virtual ~CallbackBase3() = 0;
	virtual ReturnT operator()(Param1T,Param2T,Param3T) = 0;
};

template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T> MacSim::CallbackBase3<ReturnT,Param1T,Param2T,Param3T>::~CallbackBase3() {}

template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T> 
class Callback3: public CallbackBase3<ReturnT,Param1T,Param2T,Param3T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T,Param3T);

public:
	Callback3( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback3( const Callback3<ConsumerT,ReturnT,Param1T,Param2T,Param3T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2, Param3T param3) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2,param3); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};

}}

#endif

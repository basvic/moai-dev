// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#include "pch.h"
#include <moaicore/MOAIAnim.h>
#include <moaicore/MOAIAnimCurve.h>

//================================================================//
// local
//================================================================//

//----------------------------------------------------------------//
/**	@brief <tt>( returns ) func ( self )</tt>\n
\n
	Description of method Coming Soon(tm).
	@param self (in)
	@param y (out)
*/
int MOAIAnim::_apply ( lua_State* L ) {
	LUA_SETUP ( MOAIAnim, "U" );

	float t0 = state.GetValue < float >( 2, 0.0f );
	float t1 = state.GetValue < float >( 3, 0.0f );

	self->Apply ( t0, t1 );

	return 0;
}

//----------------------------------------------------------------//
/**	@brief <tt>( length ) getLength ( self )</tt>\n
\n
	Returns the total length of the animation.
	@param self (in)
*/
int	MOAIAnim::_getLength ( lua_State* L ) {
	LUA_SETUP ( MOAIAnim, "U" )
	
	lua_pushnumber ( state, self->GetLength ());
	
	return 1;
}

//----------------------------------------------------------------//
/**	@brief <tt>reserveLinks ( self, totalLinks )</tt>\n
\n
	Reserves links for the AnimPlayer.
	@param self (in)
	@param totalLinks (in) Total number of links to be reserved.
*/
int	MOAIAnim::_reserveLinks ( lua_State* L ) {
	LUA_SETUP ( MOAIAnim, "UN" );
	
	u32 totalLinks = state.GetValue < u32 >( 2, 0 );
	self->ReserveLinks ( totalLinks );
	
	return 0;
}

//----------------------------------------------------------------//
/**	@brief <tt>setLink ( self, linkID, curveID, target, attrID )</tt>\n
\n
	Sets a link from a curve to an attribute (must be bound with the bind function).
	@param self (in)
	@param linkID (in) ID of the link index.
	@param curveID (in) ID of the curve.
	@param target (in) Target object to animate.
	@param attrID (in) ID of the attribute.
*/
int	MOAIAnim::_setLink ( lua_State* L ) {
	LUA_SETUP ( MOAIAnim, "UNUUN" );
	
	MOAINode* target = state.GetLuaObject < MOAINode >( 4 );
	if ( !target ) return 0;
	
	u32 linkID				= state.GetValue < u32 >( 2, 1 ) - 1;
	MOAIAnimCurve* curve	= state.GetLuaObject < MOAIAnimCurve >( 3 );
	u32 attrID				= state.GetValue < u32 >( 5, 0 );
	bool relative			= state.GetValue < bool >( 6, false );
	
	self->SetLink ( linkID, curve, target, attrID, relative );
	
	return 0;
}

//================================================================//
// MOAIAnim
//================================================================//

//----------------------------------------------------------------//
void MOAIAnim::Apply ( float t ) {
	
	u32 total = this->mLinks.Size ();
	for ( u32 i = 0; i < total; ++i ) {
		
		MOAIAnimLink& link = this->mLinks [ i ];
		MOAIAnimCurve* curve = link.mCurve;
		MOAINode* target = link.mTarget;
		
		if ( curve && target ) {
			
			if ( !link.mRelative ) {
				float value = curve->GetFloatValue ( t );
				target->SetAttributeValue < float >( link.mAttrID, value );
			}
			target->ScheduleUpdate ();
		}
	}
}

//----------------------------------------------------------------//
void MOAIAnim::Apply ( float t0, float t1 ) {
	
	if ( t0 == t1 ) {
		this->Apply ( t0 );
		return;
	}
	
	USAttrAdder adder;
	
	u32 total = this->mLinks.Size ();
	for ( u32 i = 0; i < total; ++i ) {
		
		MOAIAnimLink& link = this->mLinks [ i ];
		MOAIAnimCurve* curve = link.mCurve;
		MOAINode* target = link.mTarget;
		
		if ( curve && target ) {
			
			if ( link.mRelative ) {
				float value = curve->GetFloatDelta ( t0, t1 );
				adder.Set ( value );
				target->ApplyAttrOp ( link.mAttrID, adder );
			}
			else {
				float value = curve->GetFloatValue ( t0 );
				target->SetAttributeValue < float >( link.mAttrID, value );
			}
			target->ScheduleUpdate ();
		}
	}
}

//----------------------------------------------------------------//
void MOAIAnim::Clear () {

	this->mLinks.Clear ();
	this->mLength = 0.0f;
}

//----------------------------------------------------------------//
MOAIAnim::MOAIAnim () :
	mLength ( 0.0f ) {
	
	RTTI_SINGLE ( MOAITimer )
}

//----------------------------------------------------------------//
MOAIAnim::~MOAIAnim () {

	this->Clear ();
}

//----------------------------------------------------------------//
void MOAIAnim::OnUpdate ( float step ) {

	float t0 = this->mTime;
	float t1 = t0 + this->DoStep ( step );
	
	this->Apply ( t0, t1 );
}

//----------------------------------------------------------------//
void MOAIAnim::RegisterLuaClass ( USLuaState& state ) {

	MOAITimer::RegisterLuaClass ( state );
}

//----------------------------------------------------------------//
void MOAIAnim::RegisterLuaFuncs ( USLuaState& state ) {

	MOAITimer::RegisterLuaFuncs ( state );

	LuaReg regTable [] = {
		{ "apply",				_apply },
		{ "getLength",			_getLength },
		{ "reserveLinks",		_reserveLinks },
		{ "setLink",			_setLink },
		{ NULL, NULL }
	};

	luaL_register ( state, 0, regTable );
}

//----------------------------------------------------------------//
void MOAIAnim::ReserveLinks ( u32 totalLinks ) {

	this->mLinks.Init ( totalLinks );
}

//----------------------------------------------------------------//
void MOAIAnim::SetLink ( u32 linkID, MOAIAnimCurve* curve, MOAINode* target, u32 attrID, bool relative ) {

	if ( linkID >= this->mLinks.Size ()) return;
	if ( !target ) return;
	if ( !target->AttrExists ( attrID )) return;

	MOAIAnimLink& link = this->mLinks [ linkID ];
	link.mCurve		= curve;
	link.mTarget	= target;
	link.mAttrID	= attrID;
	link.mRelative	= relative;
	
	float length = curve->GetLength ();

	if ( this->mLength < length ) {
		this->mLength = length;
	}
	this->mEndTime = this->mLength;
}

//----------------------------------------------------------------//
STLString MOAIAnim::ToString () {

	STLString repr;

	PRETTY_PRINT ( repr, mLength )
	PRETTY_PRINT ( repr, mLinks )

	return repr;
}

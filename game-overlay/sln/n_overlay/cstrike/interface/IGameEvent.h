/* 
 * ModSharp
 * Copyright (C) 2023-2026 Kxnrl. All Rights Reserved.
 *
 * This file is part of ModSharp.
 * ModSharp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ModSharp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ModSharp. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CSTRIKE_INTERFACE_IGAMEEVENT_H
#define CSTRIKE_INTERFACE_IGAMEEVENT_H

#include "cstrike/type/CHashKey.h"

#include <cstdint>

class CEntityInstance;
struct KeyValues3;

using GameEventKeySymbol_t = CHashKey;

#ifdef CreateEvent
#    undef CreateEvent
#endif

class IGameEvent
{
public:
    virtual ~IGameEvent() {};
    virtual const char* GetName() const = 0; // get event name
    virtual int         GetID() const   = 0;

    virtual bool IsReliable() const                             = 0; // if event handled reliable
    virtual bool IsLocal() const                                = 0; // if event is never networked
    virtual bool IsEmpty(const GameEventKeySymbol_t& keySymbol) = 0; // check if data field exists

    // Data access
    virtual bool        GetBool(const GameEventKeySymbol_t& keySymbol, bool defaultValue = false)       = 0;
    virtual int         GetInt(const GameEventKeySymbol_t& keySymbol, int defaultValue = 0)             = 0;
    virtual uint64_t    GetUint64(const GameEventKeySymbol_t& keySymbol, uint64_t defaultValue = 0)     = 0;
    virtual float       GetFloat(const GameEventKeySymbol_t& keySymbol, float defaultValue = 0.0f)      = 0;
    virtual const char* GetString(const GameEventKeySymbol_t& keySymbol, const char* defaultValue = "") = 0;
    virtual void*       GetPtr(const GameEventKeySymbol_t& keySymbol)                                   = 0;

    virtual uint32_t GetEHandle(const GameEventKeySymbol_t& keySymbol, uint32_t defaultValue = uint32_t()) = 0;

    // Returns the entity instance, mostly used for _pawn keys, might return 0 if used on any other key (even on a controller).
    virtual CEntityInstance* GetEntity(const GameEventKeySymbol_t& keySymbol, CEntityInstance* fallbackInstance = nullptr) = 0;
    virtual int32_t          GetEntityIndex(const GameEventKeySymbol_t& keySymbol, int32_t defaultValue = (-1))         = 0;

    virtual int32_t GetPlayerSlot(const GameEventKeySymbol_t& keySymbol) = 0;

    virtual CEntityInstance* GetPlayerController(const GameEventKeySymbol_t& keySymbol) = 0;
    virtual CEntityInstance* GetPlayerPawn(const GameEventKeySymbol_t& keySymbol)       = 0;

    // Returns the EHandle for the _pawn entity.
    virtual uint32_t GetPawnEHandle(const GameEventKeySymbol_t& keySymbol) = 0;
    // Returns the int32_t for the _pawn entity.
    virtual int32_t GetPawnEntityIndex(const GameEventKeySymbol_t& keySymbol) = 0;

    virtual void SetBool(const GameEventKeySymbol_t& keySymbol, bool value)          = 0;
    virtual void SetInt(const GameEventKeySymbol_t& keySymbol, int value)            = 0;
    virtual void SetUint64(const GameEventKeySymbol_t& keySymbol, uint64_t value)    = 0;
    virtual void SetFloat(const GameEventKeySymbol_t& keySymbol, float value)        = 0;
    virtual void SetString(const GameEventKeySymbol_t& keySymbol, const char* value) = 0;
    virtual void SetPtr(const GameEventKeySymbol_t& keySymbol, void* value)          = 0;

    virtual void SetEntity(const GameEventKeySymbol_t& keySymbol, CEntityInstance* value) = 0;
    virtual void SetEntity(const GameEventKeySymbol_t& keySymbol, int32_t value)          = 0;

    // Also sets the _pawn key (Expects pawn entity to be passed)
    virtual void SetPlayer(const GameEventKeySymbol_t& keySymbol, CEntityInstance* pawn) = 0;
    // Also sets the _pawn key
    virtual void SetPlayer(const GameEventKeySymbol_t& keySymbol, int32_t value) = 0;

    // Expects pawn entity to be passed, will set the controller entity as a controllerKeyName
    // and pawn entity as a pawnKeyName.
    virtual void SetPlayerRaw(const GameEventKeySymbol_t& controllerKeySymbol, const GameEventKeySymbol_t& pawnKeySymbol, CEntityInstance* pawn) = 0;

    virtual bool HasKey(const GameEventKeySymbol_t& keySymbol) = 0;

    // Something script vm related
    virtual void unk001() = 0;

    virtual KeyValues3* GetDataKeys() const = 0;
};

class IGameEventListener2
{
public:
    virtual ~IGameEventListener2(void) {}

    // FireEvent is called by EventManager if event just occured
    // KeyValue memory will be freed by manager if not needed anymore
    virtual void FireGameEvent(IGameEvent* event) = 0;

    virtual int GetEventDebugID(void) = 0;
};

class IGameEventManager2
{
public:
    virtual ~IGameEventManager2(void) {}

    // load game event descriptions from a file eg "resource\gameevents.res"
    virtual int LoadEventsFromFile(const char* filename, bool bSearchAll) = 0;

    // removes all and anything
    virtual void Reset() = 0;

    // adds a listener for a particular event, return the event id, or -1 if invalid event name
    virtual int32_t AddListener(IGameEventListener2* listener, const char* name, bool bServerSide) = 0;

    // returns true if this listener is listens to given event
    virtual bool FindListener(IGameEventListener2* listener, const char* name) = 0;

    // removes a listener
    virtual void RemoveListener(IGameEventListener2* listener) = 0;

    // create an event by name, but doesn't fire it. returns nullptr is event is not
    // known or no listener is registered for it. bForce forces the creation even if no listener is active
    virtual IGameEvent* CreateEvent(const char* name, bool bForce = false, int* pCookie = nullptr) = 0;

    // fires a server event created earlier, if bDontBroadcast is set, event is not send to clients
    virtual bool FireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;

    // fires an event for the local client only, should be used only by client code
    virtual bool FireEventClientSide(IGameEvent* event) = 0;

    // create a new copy of this event, must be free later
    virtual IGameEvent* DuplicateEvent(IGameEvent* event) = 0;

    // if an event was created but not fired for some reason, it has to bee freed, same UnserializeEvent
    virtual void FreeEvent(IGameEvent* event) = 0;

    // write/read event to/from bitbuffer
    virtual bool        SerializeEvent(IGameEvent* event, /*CNetMessagePB<CMsgSource1LegacyGameEvent>**/ void* ev) = 0;
    virtual IGameEvent* UnserializeEvent(const /*CNetMessagePB<CMsgSource1LegacyGameEvent>&*/ void* ev)            = 0; // create new KeyValues, must be deleted

    virtual int LookupEventId(const char* name) = 0;

    virtual void PrintEventToString(IGameEvent* event, /*CUtlString**/ char** out) = 0;

    virtual bool HasEventDescriptor(const char* name) = 0;
};

#endif
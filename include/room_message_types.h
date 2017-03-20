// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//

#pragma once

#include "MessageIdentifiers.h"

enum RoomMessageTypes
{
    ID_ROOM_JOIN_RJEQUEST = ID_USER_PACKET_ENUM + 1,
    ID_ROOM_JOIN_SUCCESS,
    ID_ROOM_INFORMATION,
    ID_ROOM_MEMBER_LIST,
    ID_ROOM_SET_GAME_NAME,
    ID_ROOM_WIFI_PACKET,
    ID_ROOM_CHAT,
    ID_ROOM_NAME_COLLISION,
    ID_ROOM_MAC_COLLISION
};

#ifndef ENUMS_H
#define ENUMS_H

#pragma once
#include <string>

enum UserRequest
{
    LOGIN_REQ = 1,
    LOGOUT_REQ = 2,
    REGISTER_REQ = 3,
    VIEW_ROOMS_REQ = 4,
    CREATE_ROOM_REQ = 5,
    VIEW_ROOMS_OWNED_REQ = 6,
    VIEW_ROOM_LOG_REQ = 7,
    PLACE_ITEM_IN_ROOM_REQ = 8,
    ACCEPT_REJECT_ITEM_REQ = 9,
    VIEW_ITEMS_IN_ROOM_REQ = 10,
    DELETE_ITEM_IN_ROOM_REQ = 11,
    JOIN_ROOM_REQ = 12,
    LEAVE_ROOM_REQ = 13,
    VIEW_ALL_ITEMS_REQ = 14,
    SEARCH_ITEM_REQ = 15,
    CREATE_ITEM_REQ = 16,
    PLACE_BID_REQ = 17,
    BUY_NOW_REQ = 18,
    DELETE_ITEM_FROM_ROOM_REQ = 19,
    DELETE_ITEM_REQ = 20,
    UPDATE_ITEM_REQ = 21,
    VIEW_USER_LOG_REQ = 22,
    VIEW_OWNED_ITEMS_REQ = 23,
};

enum ServerResponse
{
    LOGIN_RES = 1,
    LOGOUT_RES = 2,
    REGISTER_RES = 3,
    VIEW_ROOMS_RES = 4,
    CREATE_ROOM_RES = 5,
    VIEW_ROOMS_OWNED_RES = 6,
    VIEW_ROOM_LOG_RES = 7,
    PLACE_ITEM_IN_ROOM_RES = 8,
    ACCEPT_REJECT_ITEM_RES = 9,
    VIEW_ITEMS_IN_ROOM_RES = 10,
    DELETE_ITEM_IN_ROOM_RES = 11,
    JOIN_ROOM_RES = 12,
    LEAVE_ROOM_RES = 13,
    VIEW_ALL_ITEMS_RES = 14,
    SEARCH_ITEM_RES = 15,
    CREATE_ITEM_RES = 16,
    PLACE_BID_RES = 17,
    BUY_NOW_RES = 18,
    DELETE_ITEM_FROM_ROOM_RES = 19,
    DELETE_ITEM_RES = 20,
    UPDATE_ITEM_RES = 21,
    VIEW_USER_LOG_RES = 22,
    VIEW_OWNED_ITEMS_RES = 23,
    WRONG_COMMAND_RES = -1
};

enum StatusCode
{
    FAIL = 0,
    SUCCESS = 1
};

#endif // ENUMS_H

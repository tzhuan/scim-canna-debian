/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __SCIM_CANNA_PREFS_H__
#define __SCIM_CANNA_PREFS_H__

#define Uses_SCIM_UTILITY
#include <scim_utility.h>

#define SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME         "/IMEngine/Canna/SpecifyInitFileName"
#define SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME            "/IMEngine/Canna/SpecifyServerName"
#define SCIM_CANNA_CONFIG_INIT_FILE_NAME                 "/IMEngine/Canna/InitFileName"
#define SCIM_CANNA_CONFIG_SERVER_NAME                    "/IMEngine/Canna/ServerName"
#define SCIM_CANNA_CONFIG_ON_OFF                         "/IMEngine/Canna/OnOff"
#define SCIM_CANNA_CONFIG_ON_OFF_KEY                     "/IMEngine/Canna/OnOffKey"

#define SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME_DEFAULT false
#define SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME_DEFAULT    false
#define SCIM_CANNA_CONFIG_INIT_FILE_NAME_DEFAULT         (scim_get_home_dir() + String("/.canna"))
#define SCIM_CANNA_CONFIG_SERVER_NAME_DEFAULT            "localhost"
#define SCIM_CANNA_CONFIG_ON_OFF_DEFAULT                 "On"
#define SCIM_CANNA_CONFIG_ON_OFF_KEY_DEFAULT             "Zenkaku_Hankaku,Shift+space"

#endif /* __SCIM_CANNA_PREFS_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/

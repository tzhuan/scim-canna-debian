/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2004 - 2005 Takuro Ashie <ashie@homa.ne.jp>
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

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <scim.h>
#include "scim_canna_imengine_factory.h"
#include "scim_canna_imengine.h"
#include "scim_canna_prefs.h"
#include "intl.h"

#define scim_module_init canna_LTX_scim_module_init
#define scim_module_exit canna_LTX_scim_module_exit
#define scim_imengine_module_init canna_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory canna_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_CANNA_UUID     "/IMEngine/Canna/UUID-"

#ifndef SCIM_CANNA_ICON_FILE
    #define SCIM_CANNA_ICON_FILE           (SCIM_ICONDIR"/scim-canna.png")
#endif

static ConfigPointer _scim_config (0);

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_CANNA_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize CANNA Engine.\n";

        _scim_config = config;

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        CannaFactory *factory = 0;

        try {
            factory = new CannaFactory (String ("ja_JP"),
                                        String ("9282dd2d-1f2d-40ad-b338-c9832a137526"),
                                        _scim_config);
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}


CannaFactory::CannaFactory (const String &lang,
                            const String &uuid,
                            const ConfigPointer &config)
    : m_uuid                   (uuid),
      m_config                 (config),
      m_specify_init_file_name (SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME_DEFAULT),
      m_specify_server_name    (SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME_DEFAULT),
      m_init_file_name         (SCIM_CANNA_CONFIG_INIT_FILE_NAME_DEFAULT),
      m_server_name            (SCIM_CANNA_CONFIG_SERVER_NAME_DEFAULT),
      m_on_off                 (SCIM_CANNA_CONFIG_ON_OFF_DEFAULT)
{
    SCIM_DEBUG_IMENGINE(1) << "Create Canna Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    scim_string_to_key_list (m_on_off_key, SCIM_CANNA_CONFIG_ON_OFF_KEY_DEFAULT);

    if (lang.length () >= 2)
        set_languages (lang);

    // config
    reload_config (m_config);
    m_reload_signal_connection
        = m_config->signal_connect_reload (slot (this, &CannaFactory::reload_config));
}

CannaFactory::~CannaFactory ()
{
    m_reload_signal_connection.disconnect ();
}

WideString
CannaFactory::get_name () const
{
    return utf8_mbstowcs (String ("Canna"));
}

WideString
CannaFactory::get_authors () const
{
    const char *package =
        PACKAGE "-" PACKAGE_VERSION "\n"
        "\n";
    const char *authors =
        _("Authors of scim-canna:\n"
          "  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>\n"
          "  Copyright (C) 2004 Hiroyuki Ikezoe <poincare@ikezoe.net>\n"
          "  \n"
          "Authors of Canna:\n"
          "  Copyright (C) 1990-1997 NEC Corporation, Tokyo, Japan.\n"
          "  Copyright (C) 2002-2004 Canna Project.\n");

    return utf8_mbstowcs (package) + utf8_mbstowcs (authors);
}

WideString
CannaFactory::get_credits () const
{
    return utf8_mbstowcs (
        _("Art work:\n"
          "  SHIMODA Hiroshi <piro@p.club.ne.jp>"));
}

WideString
CannaFactory::get_help () const
{
    const char *title = 
        _("Basic operation:\n"
          "  \n");

    const char *text1 = 
        _("1. Switch input mode:\n"
          "  You can toggle on/off Japanese mode by pressing Zenkaku_Hankaku key or\n"
          "  Shift+Space.\n"
          "  \n");

    const char *text2 = 
        _("2. Input hiragana and katakana:\n"
          "  You can input hiragana by inputting romaji. The preedit string can be\n"
          "  converted to katakana or alphabet by pressing Control+N or Control+P.\n"
          "  If you want to cancel inputting, please press Control+G\n"
          "  \n");

    const char *text3 = 
        _("3. Convert to kanji:\n"
          "  After inputting hiragana, you can convert it to kanji by pressing Space\n"
          "  key. When you press Space key once again, available candidates will be\n"
          "  shown. Press Space or Control+F to select a next candidate, and press\n"
          "  Control+B to select a previous candidate. Press Control+G to hide\n"
          "  candidates. Then you can commit the preedit string by pressing Enter\n"
          "  key or Control+M.\n"
          "  \n");

    const char *text4 = 
        _("4. Modify sentence segments:\n"
          "  After converting to kanji and before showing candidates or commit, you\n"
          "  can modify sentence segments. Press left and right cursor key or\n"
          "  Control+F and Control+B to select a next or previous segment. Press\n"
          "  Control+I or Control+O to shrink or extend the selected segment.\n"
          "  \n");

    const char *text5 = 
        _("5. Additional features:\n"
          "  You can access to additional features of Canna by pressing Home key.\n"
          "  It includes searching kanji letters, registering a word and environment\n"
          "  preferences.\n"
          "  \n");

    return utf8_mbstowcs (title)
        + utf8_mbstowcs (text1)
        + utf8_mbstowcs (text2)
        + utf8_mbstowcs (text3)
        + utf8_mbstowcs (text4)
        + utf8_mbstowcs (text5);
}

String
CannaFactory::get_uuid () const
{
    return m_uuid;
}

String
CannaFactory::get_icon_file () const
{
    return String (SCIM_CANNA_ICON_FILE);
}

IMEngineInstancePointer
CannaFactory::create_instance (const String &encoding, int id)
{
    return new CannaInstance (this, encoding, id);
}

void
CannaFactory::reload_config (const ConfigPointer &config)
{
    if (!config) return;

    String str;

    m_specify_init_file_name
        = config->read (String (SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME),
                        SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME_DEFAULT);

    m_specify_server_name
        = config->read (String (SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME),
                        SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME_DEFAULT);

    m_init_file_name
        = config->read (String (SCIM_CANNA_CONFIG_INIT_FILE_NAME),
                        SCIM_CANNA_CONFIG_INIT_FILE_NAME_DEFAULT);

    m_server_name
        = config->read (String (SCIM_CANNA_CONFIG_SERVER_NAME),
                        String (SCIM_CANNA_CONFIG_SERVER_NAME_DEFAULT));

    m_on_off
        = config->read (String (SCIM_CANNA_CONFIG_ON_OFF),
                        String (SCIM_CANNA_CONFIG_ON_OFF_DEFAULT));

    str = config->read (String (SCIM_CANNA_CONFIG_ON_OFF_KEY),
                        String (SCIM_CANNA_CONFIG_ON_OFF_KEY_DEFAULT));
    scim_string_to_key_list (m_on_off_key, str);
}

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
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <scim.h>
#include "scim_canna_imengine_factory.h"
#include "scim_canna_imengine.h"
#include "scim_canna_prefs.h"
#include "intl.h"

CannaInstance::CannaInstance (CannaFactory   *factory,
                              const String   &encoding,
                              int             id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_prev_key (0,0),
      m_canna_jrkanji (this)
{
    SCIM_DEBUG_IMENGINE(1) << "Create CANNA Instance : ";
}

CannaInstance::~CannaInstance ()
{
}

bool
CannaInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    KeyEvent newkey;

    // ignore key release.
    if (key.is_key_release ()) {
        return true;
    }

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;

    if (m_canna_jrkanji.process_key_event (key))
        return true;

    return false;
}

void
CannaInstance::move_preedit_caret (unsigned int pos)
{
}

void
CannaInstance::select_candidate (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    select_candidate_no_direct (item);
}

void
CannaInstance::select_candidate_no_direct (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate_no_direct.\n";

    m_lookup_table.set_cursor_pos_in_current_page (item);
    update_lookup_table (m_lookup_table);
}

void
CannaInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
CannaInstance::lookup_table_page_up ()
{
    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();
    update_lookup_table (m_lookup_table);
}

void
CannaInstance::lookup_table_page_down ()
{
    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();
    update_lookup_table (m_lookup_table);
}

void
CannaInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";

    m_canna_jrkanji.reset ();

    update_preedit_caret (0);
    update_preedit_string (utf8_mbstowcs (""));
    update_aux_string (utf8_mbstowcs (""));
    m_lookup_table.clear ();
    hide_preedit_string ();
    hide_lookup_table ();
    hide_aux_string ();
}

void
CannaInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    register_properties (m_canna_jrkanji.get_properties ());

    if (m_canna_jrkanji.preedit_string_visible ())
        m_canna_jrkanji.show_preedit_string ();
    if (m_canna_jrkanji.aux_string_visible ())
        m_canna_jrkanji.show_aux_string ();
}

void
CannaInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";

    reset ();
}

void
CannaInstance::trigger_property (const String &property)
{
    String canna_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << " - " << canna_prop << "\n";

    m_canna_jrkanji.trigger_property (property);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/

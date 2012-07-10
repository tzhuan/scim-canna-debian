/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 - 2005 Takuro Ashie
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

#ifndef __SCIM_CANNA_IMENGINE_H__
#define __SCIM_CANNA_IMENGINE_H__

#include <scim.h>
#include <canna_jrkanji.h>
#include <scim_canna_imengine_factory.h>

using namespace scim;

class CannaInstance : public IMEngineInstanceBase
{
    friend class CannaFactory;
    friend class CannaJRKanji;

private:
    CannaFactory           *m_factory;

    KeyEvent                m_prev_key;

    /* for candidates window */
    CommonLookupTable       m_lookup_table;

    /* for toolbar */
    PropertyList            m_properties;

    /* backend */
    CannaJRKanji            m_canna_jrkanji;

    /* flags */
    //

public:
    CannaInstance (CannaFactory   *factory,
                   const String   &encoding,
                   int             id = -1);
    virtual ~CannaInstance ();

    virtual bool process_key_event             (const KeyEvent& key);
    virtual void move_preedit_caret            (unsigned int pos);
    virtual void select_candidate              (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up          (void);
    virtual void lookup_table_page_down        (void);
    virtual void reset                         (void);
    virtual void focus_in                      (void);
    virtual void focus_out                     (void);
    virtual void trigger_property              (const String &property);

public:

private: // actions

private:
    void   select_candidate_no_direct          (unsigned int    item);
};
#endif /* __SCIM_CANNA_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/

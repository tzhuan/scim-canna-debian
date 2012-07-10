/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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

#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <string.h>
#include "canna_jrkanji.h"
#include "scim_canna_imengine.h"
#include "scim_canna_imengine_factory.h"
#include "intl.h"

#define SCIM_PROP_INPUT_MODE                 "/IMEngine/Canna/InputMode"
#define SCIM_PROP_INPUT_MODE_OFF             "/IMEngine/Canna/InputMode/Off"
#define SCIM_PROP_INPUT_MODE_HIRAGANA        "/IMEngine/Canna/InputMode/Hiragana"
#define SCIM_PROP_INPUT_MODE_KATAKANA        "/IMEngine/Canna/InputMode/Katakana"
#define SCIM_PROP_INPUT_MODE_HALF_KATAKANA   "/IMEngine/Canna/InputMode/HalfKatakana"
#define SCIM_PROP_INPUT_MODE_ALPHABET        "/IMEngine/Canna/InputMode/Alphabet"
#define SCIM_PROP_INPUT_MODE_WIDE_ALPHABET   "/IMEngine/Canna/InputMode/WideAlphabet"

#define SCIM_PROP_INPUT_MODE_KIGO            "/IMEngine/Canna/InputMode/Kigo"
#define SCIM_PROP_INPUT_MODE_HEX             "/IMEngine/Canna/InputMode/Hex"
#define SCIM_PROP_INPUT_MODE_BUSHU           "/IMEngine/Canna/InputMode/Bushu"

static unsigned int n_instance = 0;
static unsigned int last_created_context_id = 0;

CannaJRKanji::CannaJRKanji (CannaInstance *ci)
    : m_canna (ci),
      m_enabled (false),
      m_context_id (last_created_context_id++),
      m_preediting (false),
      m_aux_string_visible (false)
{
    char **warn = NULL, **p;

    if (m_canna->m_factory->m_on_off == "On") {
        m_enabled = true;
    } else if (m_canna->m_factory->m_on_off == "Off") {
        m_enabled = false;
    } else {
        m_enabled = false;
    }

    m_iconv.set_encoding ("EUC-JP");

    // initialize canna library
    if (n_instance == 0) {
        if (m_canna->m_factory->m_specify_init_file_name) {
            const char *file = m_canna->m_factory->m_init_file_name.c_str();
            jrKanjiControl (0, KC_SETINITFILENAME, (char *) file);
        }

        if (m_canna->m_factory->m_specify_server_name) {
            const char *server = m_canna->m_factory->m_server_name.c_str();
            jrKanjiControl (0, KC_SETSERVERNAME, (char *) server);
        }

        jrKanjiControl (0, KC_INITIALIZE, (char *) &warn);

        for (p = warn; warn && *p; p++) {
            // error
        }

        jrKanjiControl (0, KC_SETAPPNAME, "scim-canna");
    }

    // initialize canna context
    m_workbuf[0]       = '\0';
    m_ksv.ks           = &m_ks;
    m_ksv.buffer       = m_workbuf;
    m_ksv.bytes_buffer = CANNA_MAX_SIZE;
    m_ksv.val          = CANNA_MODE_HenkanMode;
    jrKanjiControl (m_context_id, KC_CHANGEMODE, (char *) &m_ksv);

    n_instance++;

    // set mode line
    install_properties ();
    set_mode_line ();
}

CannaJRKanji::~CannaJRKanji ()
{
    jrKanjiControl (m_context_id, KC_CLOSEUICONTEXT, (char *) &m_ksv);

    if (n_instance != 0) {
        n_instance--;
        if (n_instance == 0)
            jrKanjiControl (0, KC_FINALIZE, NULL);
    }
}

bool
match_key_event (const KeyEventList &list, const KeyEvent &key,
                 uint16 ignore_mask)
{
    KeyEventList::const_iterator kit;

    for (kit = list.begin (); kit != list.end (); kit++) {
        uint16 mod1, mod2;

        mod1 = kit->mask;
        mod2 = key.mask;
        mod1 &= ~ignore_mask;
        mod2 &= ~ignore_mask;

        if (key.code == kit->code && mod1 == mod2)
             return true;
    }
    return false;
}

void
CannaJRKanji::install_properties (void)
{
    m_properties.clear ();

    Property prop;

    prop = Property (SCIM_PROP_INPUT_MODE,
                     _("Input mode"), String (""),
                     _("Input mode"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_OFF,
                     _("Off"), String (""),
                     _("Off"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_HIRAGANA,
                     _("Hiragana"), String (""),
                     _("Hiragana"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_KATAKANA,
                     _("Katakana"), String (""),
                     _("Katakana"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_HALF_KATAKANA,
                     _("Half width katakana"), String (""),
                     _("Half width katakana"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_ALPHABET,
                     _("Alphabet"), String (""),
                     _("Alphabet"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_WIDE_ALPHABET,
                     _("Wide alphabet"), String (""),
                     _("Wide alphabet"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_KIGO,
                     _("Symbols"), String (""),
                     _("Search a symbol"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_HEX,
                     _("Hex"), String (""),
                     _("Search a kanji by hex"));
    m_properties.push_back (prop);

    prop = Property (SCIM_PROP_INPUT_MODE_BUSHU,
                     _("Bushu"), String (""),
                     _("Search a kanji by bushu"));
    m_properties.push_back (prop);
}

int
CannaJRKanji::translate_key_event (const KeyEvent &key)
{
    switch (key.code) {
    case SCIM_KEY_Return:
        return 0x0d;

    case SCIM_KEY_BackSpace:
        return 0x08;

    case SCIM_KEY_Insert:
        return CANNA_KEY_Insert;

    case SCIM_KEY_Up:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Up;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Up;
        }
        return  CANNA_KEY_Up;

    case SCIM_KEY_Down:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Down;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Down;
        }
        return  CANNA_KEY_Down;

    case SCIM_KEY_Left:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Left;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Left;
        }
        return  CANNA_KEY_Left;

    case SCIM_KEY_Right:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Right;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Right;
        }
        return  CANNA_KEY_Right;

    case SCIM_KEY_Page_Up:
        return CANNA_KEY_Rolldown;

    case SCIM_KEY_Page_Down:
        return CANNA_KEY_Rollup;

    case SCIM_KEY_Home:
        return CANNA_KEY_Home;

    case SCIM_KEY_Help:
    case SCIM_KEY_Escape:
        return CANNA_KEY_Help;

    case SCIM_KEY_F1:
        return CANNA_KEY_F1;

    case SCIM_KEY_F2:
        return CANNA_KEY_F2;

    case SCIM_KEY_F3:
        return CANNA_KEY_F3;

    case SCIM_KEY_F4:
        return CANNA_KEY_F4;

    case SCIM_KEY_F5:
        return CANNA_KEY_F5;

    case SCIM_KEY_F6:
        return CANNA_KEY_F6;

    case SCIM_KEY_F7:
        return CANNA_KEY_F7;

    case SCIM_KEY_F8:
        return CANNA_KEY_F8;

    case SCIM_KEY_F9:
        return CANNA_KEY_F9;

    case SCIM_KEY_F10:
        return CANNA_KEY_F10;

    case SCIM_KEY_Henkan_Mode:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Xfer;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Xfer;
        }
        return CANNA_KEY_Xfer;

    case SCIM_KEY_Muhenkan:
        if (key.mask & SCIM_KEY_ControlMask) {
            return CANNA_KEY_Cntrl_Nfer;
        } else if (key.mask & SCIM_KEY_ShiftMask) {
            return CANNA_KEY_Shift_Nfer;
        }
        return CANNA_KEY_Nfer;

    default:
        if (key.code >= SCIM_KEY_a && key.code <= SCIM_KEY_z){
            if (key.mask & SCIM_KEY_ControlMask)
                return key.code - SCIM_KEY_a + 1;
        }

        if (key.code == SCIM_KEY_at) {
            if (key.code & SCIM_KEY_ControlMask)
                return 0;
        }

        return key.get_ascii_code ();
    }

    return 0xffff;
}

unsigned int
CannaJRKanji::convert_string (WideString &dest,
                              AttributeList &attr_list,
                              const char *str,
                              unsigned int len,
                              unsigned int cur_pos,
                              unsigned int cur_len)
{
    // cut the string
    char left_str[cur_pos + 1];
    char cur_str[cur_len + 1];
    char right_str[len - cur_pos - cur_len + 1];
    strncpy (left_str, str, cur_pos);
    left_str[cur_pos] = '\0';
    strncpy (cur_str, (const char *) (str + cur_pos), cur_len);
    cur_str[cur_len] = '\0';
    strncpy (right_str, (const char *) (str + cur_pos + cur_len),
             len - cur_pos - cur_len);
    right_str[len - cur_pos - cur_len] = '\0';

    // convert
    WideString left, cur, right;
    m_iconv.convert (left,  left_str);
    m_iconv.convert (cur,   cur_str);
    m_iconv.convert (right, right_str);

    // join all string
    dest = left + cur + right;

    // set attributes
    Attribute attr (left.length (), cur.length (), SCIM_ATTR_DECORATE);
    attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
    attr_list.push_back (attr);

    return left.length ();
}

bool
CannaJRKanji::process_key_event (const KeyEvent &key)
{
    int size = 1024, n, ch;
    char buf[1024 + 1];

    if (match_key_event (m_canna->m_factory->m_on_off_key, key, 0)) {
        m_enabled = !m_enabled;
        set_mode_line ();
        m_canna->reset ();
        return true;
    }

    if (!m_enabled)
        return false;

    ch = translate_key_event (key);
    if (ch == 0xffff)
        return false;

    n = jrKanjiString (m_context_id, ch, buf, size, &m_ks);

    // commit string
    if (n > 0 && !(m_ks.info & KanjiThroughInfo)) {
        buf[n] = '\0';
        WideString dest;
        m_iconv.convert (dest, buf);
        m_canna->commit_string (dest);
    }

    // mode line string
    if (m_ks.info & KanjiModeInfo)
        set_mode_line ();

    // guide line string
    set_guide_line ();

    // preedit string
    if (m_ks.length > 0) {
        WideString dest;
        AttributeList attrs;
        unsigned int pos;
        pos = convert_string (dest, attrs,
                              (const char *) m_ks.echoStr,
                              m_ks.length,
                              m_ks.revPos,
                              m_ks.revLen);

        m_canna->update_preedit_string (dest, attrs);
        m_canna->update_preedit_caret (pos);

        if (!m_preediting && dest.length () <= 0) {
            m_canna->hide_preedit_string ();
            return !(m_ks.info & KanjiThroughInfo);
        } else {
            m_preediting = true;
            m_canna->show_preedit_string ();
            m_canna->hide_lookup_table ();
            return true;
        }

    } else if (m_ks.length == 0) {
        m_canna->update_preedit_string (utf8_mbstowcs (""));

        m_canna->hide_preedit_string ();
        m_canna->hide_lookup_table ();

        if (m_preediting) {
            m_preediting = false;
            return true;
        } else {
            return !(m_ks.info & KanjiThroughInfo);
        }
    }

    m_canna->hide_lookup_table ();

    return !(m_ks.info & KanjiThroughInfo);
}

void
CannaJRKanji::trigger_property (const String &property)
{
    int val = m_ksv.val;

    if (property == SCIM_PROP_INPUT_MODE_OFF) {
        m_enabled = false;
        set_mode_line ();
    } else if (property == SCIM_PROP_INPUT_MODE_HIRAGANA) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_ZenHiraHenkanMode;

    } else if (property == SCIM_PROP_INPUT_MODE_KATAKANA) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_ZenKataHenkanMode;

    } else if (property == SCIM_PROP_INPUT_MODE_HALF_KATAKANA) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_HanKataHenkanMode;

    } else if (property == SCIM_PROP_INPUT_MODE_ALPHABET) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_HanAlphaHenkanMode;

    } else if (property == SCIM_PROP_INPUT_MODE_WIDE_ALPHABET) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_ZenAlphaHenkanMode;

    } else if (property == SCIM_PROP_INPUT_MODE_KIGO) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_KigoMode;

    } else if (property == SCIM_PROP_INPUT_MODE_HEX) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_HexMode;

    } else if (property == SCIM_PROP_INPUT_MODE_BUSHU) {
        m_enabled = true;
        m_ksv.val = CANNA_MODE_BushuMode;
    }

    if (val != m_ksv.val) {
        jrKanjiControl (m_context_id, KC_CHANGEMODE, (char *) &m_ksv);
        set_mode_line ();
        set_guide_line ();
    }
}

void
CannaJRKanji::set_mode_line (void)
{
    if (m_enabled) {
        int max_mode_len = jrKanjiControl(m_context_id,
                                          KC_QUERYMAXMODESTR, 0);
        unsigned char current_mode[max_mode_len];
        jrKanjiControl(m_context_id, KC_QUERYMODE, (char *) current_mode);
        WideString dest;
        m_iconv.convert (dest, (const char *) current_mode);
        m_properties[0].set_label (utf8_wcstombs(dest).c_str());
        m_canna->register_properties (m_properties);
    } else {
        m_properties[0].set_label (_("[Off]"));
        m_canna->register_properties (m_properties);
    }
}

void
CannaJRKanji::set_guide_line (void)
{
    if (m_ks.info & KanjiGLineInfo) {
        WideString dest;
        AttributeList attrs;
        convert_string (dest, attrs,
                        (const char *) m_ks.gline.line,
                        m_ks.gline.length,
                        m_ks.gline.revPos,
                        m_ks.gline.revLen);
        m_canna->update_aux_string (dest, attrs);
        if (dest.length () > 0) {
            m_aux_string_visible = true;
            m_canna->show_aux_string ();
        } else {
            m_aux_string_visible = false;
            m_canna->hide_aux_string ();
        }
    }
}

bool
CannaJRKanji::preedit_string_visible (void)
{
    return m_preediting;
}

void
CannaJRKanji::show_preedit_string (void)
{
    if (!m_preediting)
        return;

    WideString dest;
    AttributeList attrs;
    unsigned int pos;
    pos = convert_string (dest, attrs,
                          (const char *) m_ks.echoStr,
                          m_ks.length,
                          m_ks.revPos,
                          m_ks.revLen);

    m_canna->update_preedit_string (dest, attrs);
    m_canna->update_preedit_caret (pos);

    m_canna->show_preedit_string ();
}

bool
CannaJRKanji::aux_string_visible (void)
{
    return m_aux_string_visible;
}

void
CannaJRKanji::show_aux_string (void)
{
    if (!m_aux_string_visible)
        return;

    set_guide_line ();
}

void
CannaJRKanji::reset (void)
{
    jrKanjiControl (m_context_id, KC_KILL, (char *) &m_ksv);
}

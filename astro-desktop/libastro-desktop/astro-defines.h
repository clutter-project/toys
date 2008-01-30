/*
 * Copyright (C) 2007 OpenedHand Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Neil Jagdish Patel <njp@o-hand.com>
 */


#ifndef _HAVE_ASTRO_DEFINES_H
#define _HAVE_ASTRO_DEFINES_H

#define CSW() (CLUTTER_STAGE_WIDTH())
#define CSH() (CLUTTER_STAGE_HEIGHT())

#define ASTRO_PANEL_HEIGHT() (CSH() * 0.1)

#define ASTRO_WINDOW_WIDTH() (CSW())
#define ASTRO_WINDOW_HEIGHT() (CSW()-ASTRO_PANEL_HEIGHT())

#define ASTRO_APPICON_SIZE() (CSH()*0.4)
#define ASTRO_APPICON_SPACING() (ASTRO_APPICON_SIZE()*0.9)

#define ASTRO_APPLET_HEIGHT() (CSH()*0.2)
#define ASTRO_APPLET_PADDING 4

#endif

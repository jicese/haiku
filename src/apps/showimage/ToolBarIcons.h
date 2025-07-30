/*
 * Copyright 2011 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef TOOL_BAR_ICONS_H
#define TOOL_BAR_ICONS_H


#include <SupportDefs.h>


class BBitmap;

enum {
	kIconDocumentOpen = 0,
	kIconDocumentSaveAs,
	kIconDocumentSave,
	kIconDrawRectangularSelection,
	kIconEditCopy,
	kIconEditCut,
	kIconEditDelete,
	kIconEditTrash,
	kIconMediaMovieLibrary,
	kIconMediaPlaybackStartEnabled,
	kIconGoDown,
	kIconGoNext,
	kIconGoPrevious,
	kIconGoUp,
	kIconViewFullScreen,
	kIconViewWindowed,
	kIconZoomFitBest,
	kIconZoomFitViewBest,
	kIconZoomIn,
	kIconZoomOriginal,
	kIconZoomOut,
	kIconMap,
	kIconPagePrevious,
	kIconPageNext
};


status_t init_tool_bar_icons();
void uninit_tool_bar_icons();
const BBitmap* tool_bar_icon(uint32 which);


#endif // TOOL_BAR_ICONS_H

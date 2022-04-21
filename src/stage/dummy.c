/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "dummy.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

//Dummy background structure
typedef struct
{
	//Stage background base structure
	StageBack back;




    Gfx_Tex tex_back1; //Curtains
} Back_Dummy;

//Week 1 background functions
void Back_Dummy_DrawBG(StageBack *back)
{
	Back_Dummy *this = (Back_Dummy*)back;
	
	fixed_t fx, fy;
	
	//Draw curtains
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT tex_src = {0, 0, 255, 255};
	RECT_FIXED tex_dst = {
		FIXED_DEC(0,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(256,1)
	};
	
	Gfx_DrawTexRotate(&this->tex_back1, &tex_src, &tex_dst, stage.camera.bzoom, 90);
}

//Dummy background functions
void Back_Dummy_Free(StageBack *back)
{
	Back_Dummy *this = (Back_Dummy*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Dummy_New(void)
{
	//Allocate background structure
	Back_Dummy *this = (Back_Dummy*)Mem_Alloc(sizeof(Back_Dummy));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Dummy_DrawBG;
	this->back.free = Back_Dummy_Free;
	
    //Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	//Use non-pitch black background
	Gfx_SetClear(62, 48, 64);
	
	return (StageBack*)this;
}
